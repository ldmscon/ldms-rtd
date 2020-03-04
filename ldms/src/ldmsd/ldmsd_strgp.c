/* -*- c-basic-offset: 8 -*-
 * Copyright (c) 2015-2016,2018 National Technology & Engineering Solutions
 * of Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with
 * NTESS, the U.S. Government retains certain rights in this software.
 * Copyright (c) 2015-2016,2018 Open Grid Computing, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the BSD-type
 * license below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *      Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *      Neither the name of Sandia nor the names of any contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *      Neither the name of Open Grid Computing nor the names of any
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *      Modified source versions must be plainly marked as such, and
 *      must not be misrepresented as being the original software.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <coll/rbt.h>
#include <ovis_util/util.h>
#include "ldms.h"
#include "ldmsd.h"
#include "ldmsd_plugin.h"
#include "ldmsd_store.h"
#include "ldms_xprt.h"
#include "config.h"

void ldmsd_strgp___del(ldmsd_cfgobj_t obj)
{
	ldmsd_strgp_t strgp = (ldmsd_strgp_t)obj;

	if (strgp->schema)
		free(strgp->schema);
	if (strgp->metric_arry)
		free(strgp->metric_arry);

	struct ldmsd_strgp_metric *metric;
	while (!TAILQ_EMPTY(&strgp->metric_list) ) {
		metric = TAILQ_FIRST(&strgp->metric_list);
		if (metric->name)
			free(metric->name);
		TAILQ_REMOVE(&strgp->metric_list, metric, entry);
		free(metric);
	}
	ldmsd_name_match_t match;
	while (!LIST_EMPTY(&strgp->prdcr_list)) {
		match = LIST_FIRST(&strgp->prdcr_list);
		if (match->regex_str)
			free(match->regex_str);
		regfree(&match->regex);
		LIST_REMOVE(match, entry);
		free(match);
	}
	if (strgp->inst)
		ldmsd_plugin_inst_put(strgp->inst);
	ldmsd_cfgobj___del(obj);
}

static void strgp_update_fn(ldmsd_strgp_t strgp, ldmsd_prdcr_set_t prd_set)
{
	if (strgp->state != LDMSD_STRGP_STATE_OPENED)
		return;
	ldmsd_store_store(strgp->inst, prd_set->set, strgp);
}

int store_actor(ev_worker_t src, ev_worker_t dst, ev_status_t status, ev_t ev)
{
	ldmsd_strgp_t strgp = EV_DATA(ev, struct store_data)->strgp;
	ldmsd_prdcr_set_t prd_set = EV_DATA(ev, struct store_data)->prd_set;
	strgp->update_fn(strgp, prd_set);
	ldmsd_prdcr_set_ref_put(prd_set, "store_ev");
	return 0;
}

ldmsd_strgp_t
ldmsd_strgp_new_with_auth(const char *name, uid_t uid, gid_t gid, int perm)
{
	struct ldmsd_strgp *strgp;

	ev_worker_t worker;
	ev_t start_ev, stop_ev;
	char worker_name[PATH_MAX];

	snprintf(worker_name, PATH_MAX, "strgp:%s", name);
	worker = ev_worker_new(worker_name, store_actor);
	if (!worker) {
		ldmsd_log(LDMSD_LERROR,
			  "%s: error %d creating new worker %s\n",
			  __func__, errno, worker_name);
		return NULL;
	}

	start_ev = ev_new(strgp_start_type);
	if (!start_ev) {
		ldmsd_log(LDMSD_LERROR,
			  "%s: error %d creating %s event\n",
			  __func__, errno, ev_type_name(strgp_start_type));
		return NULL;
	}

	stop_ev = ev_new(strgp_stop_type);
	if (!stop_ev) {
		ldmsd_log(LDMSD_LERROR,
			  "%s: error %d creating %s event\n",
			  __func__, errno, ev_type_name(strgp_stop_type));
		return NULL;
	}

	strgp = (struct ldmsd_strgp *)
		ldmsd_cfgobj_new_with_auth(name, LDMSD_CFGOBJ_STRGP,
				 sizeof *strgp, ldmsd_strgp___del,
				 uid, gid, perm);
	if (!strgp)
		return NULL;

	strgp->state = LDMSD_STRGP_STATE_STOPPED;
	strgp->update_fn = strgp_update_fn;
	LIST_INIT(&strgp->prdcr_list);
	TAILQ_INIT(&strgp->metric_list);

	strgp->worker = worker;
	strgp->start_ev = start_ev;
	strgp->stop_ev = stop_ev;
	EV_DATA(strgp->start_ev, struct start_data)->entity = strgp;
	EV_DATA(strgp->stop_ev, struct start_data)->entity = strgp;

	ldmsd_cfgobj_unlock(&strgp->obj);
	return strgp;
}

ldmsd_strgp_t
ldmsd_strgp_new(const char *name)
{
	struct ldmsd_sec_ctxt sctxt;
	ldmsd_sec_ctxt_get(&sctxt);
	return ldmsd_strgp_new_with_auth(name, sctxt.crd.uid, sctxt.crd.gid, 0777);
}

ldmsd_strgp_t ldmsd_strgp_first()
{
	return (ldmsd_strgp_t)ldmsd_cfgobj_first(LDMSD_CFGOBJ_STRGP);
}

ldmsd_strgp_t ldmsd_strgp_next(struct ldmsd_strgp *strgp)
{
	return (ldmsd_strgp_t)ldmsd_cfgobj_next(&strgp->obj);
}

ldmsd_strgp_metric_t ldmsd_strgp_metric_first(ldmsd_strgp_t strgp)
{
	return TAILQ_FIRST(&strgp->metric_list);
}

ldmsd_strgp_metric_t ldmsd_strgp_metric_next(ldmsd_strgp_metric_t metric)
{
	return TAILQ_NEXT(metric, entry);
}

ldmsd_name_match_t ldmsd_strgp_prdcr_first(ldmsd_strgp_t strgp)
{
	return LIST_FIRST(&strgp->prdcr_list);
}

ldmsd_name_match_t ldmsd_strgp_prdcr_next(ldmsd_name_match_t match)
{
	return LIST_NEXT(match, entry);
}

time_t convert_rotate_str(const char *rotate)
{
	char *units;
	long rotate_interval;

	rotate_interval = strtol(rotate, &units, 0);
	if (rotate_interval <= 0 || *units == '\0')
		return 0;

	switch (*units) {
	case 'm':
	case 'M':
		return rotate_interval * 60;
	case 'h':
	case 'H':
		return rotate_interval * 60 * 60;
	case 'd':
	case 'D':
		return rotate_interval * 24 * 60 * 60;
	}
	return 0;
}

ldmsd_name_match_t strgp_find_prdcr_ex(ldmsd_strgp_t strgp, const char *ex)
{
	ldmsd_name_match_t match;
	LIST_FOREACH(match, &strgp->prdcr_list, entry) {
		if (0 == strcmp(match->regex_str, ex))
			return match;
	}
	return NULL;
}

int ldmsd_strgp_prdcr_add(const char *strgp_name, const char *regex_str,
			  char *rep_buf, size_t rep_len, ldmsd_sec_ctxt_t ctxt)
{
	int rc = 0;
	ldmsd_strgp_t strgp = ldmsd_strgp_find(strgp_name);
	if (!strgp)
		return ENOENT;

	ldmsd_strgp_lock(strgp);
	rc = ldmsd_cfgobj_access_check(&strgp->obj, 0222, ctxt);
	if (rc)
		goto out_1;
	if (strgp->state != LDMSD_STRGP_STATE_STOPPED) {
		rc = EBUSY;
		goto out_1;
	}
	ldmsd_name_match_t match = calloc(1, sizeof *match);
	if (!match) {
		rc = ENOMEM;
		goto out_1;
	}
	match->regex_str = strdup(regex_str);
	if (!match->regex_str) {
		rc = ENOMEM;
		goto out_2;
	}
	rc = ldmsd_compile_regex(&match->regex, regex_str,
					rep_buf, rep_len);
	if (rc)
		goto out_3;
	match->selector = LDMSD_NAME_MATCH_INST_NAME;
	LIST_INSERT_HEAD(&strgp->prdcr_list, match, entry);
	goto out_1;
out_3:
	free(match->regex_str);
out_2:
	free(match);
out_1:
	ldmsd_strgp_unlock(strgp);
	ldmsd_strgp_put(strgp);
	return rc;
}

int ldmsd_strgp_prdcr_del(const char *strgp_name, const char *regex_str,
			ldmsd_sec_ctxt_t ctxt)
{
	int rc = 0;
	ldmsd_strgp_t strgp = ldmsd_strgp_find(strgp_name);
	if (!strgp)
		return ENOENT;

	ldmsd_strgp_lock(strgp);
	rc = ldmsd_cfgobj_access_check(&strgp->obj, 0222, ctxt);
	if (rc)
		goto out_1;
	if (strgp->state != LDMSD_STRGP_STATE_STOPPED) {
		rc = EBUSY;
		goto out_1;
	}
	ldmsd_name_match_t match = strgp_find_prdcr_ex(strgp, regex_str);
	if (!match) {
		rc = EEXIST;
		goto out_1;
	}
	LIST_REMOVE(match, entry);
	free(match->regex_str);
	regfree(&match->regex);
	free(match);
out_1:
	ldmsd_strgp_unlock(strgp);
	ldmsd_strgp_put(strgp);
	return rc;
}

ldmsd_strgp_metric_t strgp_metric_find(ldmsd_strgp_t strgp, const char *name)
{
	ldmsd_strgp_metric_t metric;
	TAILQ_FOREACH(metric, &strgp->metric_list, entry)
		if (0 == strcmp(name, metric->name))
			return metric;
	return NULL;
}

ldmsd_strgp_metric_t strgp_metric_new(const char *metric_name)
{
	ldmsd_strgp_metric_t metric = calloc(1, sizeof *metric);
	if (metric) {
		metric->name = strdup(metric_name);
		if (!metric->name) {
			free(metric);
			metric = NULL;
		}
	}
	return metric;
}

int ldmsd_strgp_metric_add(const char *strgp_name, const char *metric_name,
			   ldmsd_sec_ctxt_t ctxt)
{
	int rc = 0;
	ldmsd_strgp_t strgp = ldmsd_strgp_find(strgp_name);
	if (!strgp)
		return ENOENT;

	ldmsd_strgp_lock(strgp);
	rc = ldmsd_cfgobj_access_check(&strgp->obj, 0222, ctxt);
	if (rc)
		goto out_1;
	if (strgp->state != LDMSD_STRGP_STATE_STOPPED) {
		rc = EBUSY;
		goto out_1;
	}
	ldmsd_strgp_metric_t metric = strgp_metric_find(strgp, metric_name);
	if (metric) {
		rc = EEXIST;
		goto out_1;
	}
	metric = strgp_metric_new(metric_name);
	if (!metric) {
		rc = ENOMEM;
		goto out_1;
	}
	TAILQ_INSERT_TAIL(&strgp->metric_list, metric, entry);
out_1:
	ldmsd_strgp_unlock(strgp);
	ldmsd_strgp_put(strgp);
	return rc;
}

int ldmsd_strgp_metric_del(const char *strgp_name, const char *metric_name,
			   ldmsd_sec_ctxt_t ctxt)
{
	int rc = 0;
	ldmsd_strgp_t strgp = ldmsd_strgp_find(strgp_name);
	if (!strgp)
		return ENOENT;
	ldmsd_strgp_lock(strgp);
	rc = ldmsd_cfgobj_access_check(&strgp->obj, 0222, ctxt);
	if (rc)
		goto out_1;
	if (strgp->state != LDMSD_STRGP_STATE_STOPPED) {
		rc = EBUSY;
		goto out_1;
	}
	ldmsd_strgp_metric_t metric = strgp_metric_find(strgp, metric_name);
	if (!metric) {
		rc = EEXIST;
		goto out_1;
	}
	TAILQ_REMOVE(&strgp->metric_list, metric, entry);
	free(metric->name);
	free(metric);
out_1:
	ldmsd_strgp_unlock(strgp);
	ldmsd_strgp_put(strgp);
	return rc;
}

static ldmsd_strgp_ref_t strgp_ref_new(ldmsd_strgp_t strgp, ldmsd_prdcr_set_t prd_set)
{
	ldmsd_strgp_ref_t ref = calloc(1, sizeof *ref);
	if (ref) {
		ref->strgp = ldmsd_strgp_get(strgp);
		ref->store_ev = ev_new(prdcr_set_store_type);
		EV_DATA(ref->store_ev, struct store_data)->strgp = strgp;
		EV_DATA(ref->store_ev, struct store_data)->prd_set = prd_set;
	}
	return ref;
}

static ldmsd_strgp_ref_t strgp_ref_find(ldmsd_prdcr_set_t prd_set, ldmsd_strgp_t strgp)
{
	ldmsd_strgp_ref_t ref;
	LIST_FOREACH(ref, &prd_set->strgp_list, entry) {
		if (ref->strgp == strgp)
			return ref;
	}
	return NULL;
}

static void strgp_close(ldmsd_strgp_t strgp)
{
	ldmsd_store_close(strgp->inst);
}

static int strgp_open(ldmsd_strgp_t strgp, ldmsd_prdcr_set_t prd_set)
{
	int i, idx, rc;
	const char *name;
	ldmsd_strgp_metric_t metric;

	if (!prd_set->set)
		return ENOENT;

	/* Build metric list from the schema in the producer set */
	strgp->metric_count = 0;
	strgp->metric_arry = calloc(ldms_set_card_get(prd_set->set), sizeof(int));
	if (!strgp->metric_arry)
		return ENOMEM;

	rc = ENOMEM;
	if (TAILQ_EMPTY(&strgp->metric_list)) {
		/* No metric list was given. Add all metrics in the set */
		for (i = 0; i < ldms_set_card_get(prd_set->set); i++) {
			name = ldms_metric_name_get(prd_set->set, i);
			metric = strgp_metric_new(name);
			if (!metric)
				goto err;
			TAILQ_INSERT_TAIL(&strgp->metric_list, metric, entry);
		}
	}
	rc = ENOENT;
	for (i = 0, metric = ldmsd_strgp_metric_first(strgp);
	     metric; metric = ldmsd_strgp_metric_next(metric), i++) {
		name = metric->name;
		idx = ldms_metric_by_name(prd_set->set, name);
		if (idx < 0)
			goto err;
		metric->idx = idx;
		metric->type = ldms_metric_type_get(prd_set->set, idx);
		strgp->metric_arry[i] = idx;
	}
	strgp->metric_count = i;
	rc = ldmsd_store_open(strgp->inst, strgp);
	if (rc)
		goto err;
	return 0;
err:
	free(strgp->metric_arry);
	strgp->metric_arry = NULL;
	strgp->metric_count = 0;
	return rc;
}

/** Must be called with the producer set lock and the strgp config lock held and in this order*/
int ldmsd_strgp_update_prdcr_set(ldmsd_strgp_t strgp, ldmsd_prdcr_set_t prd_set)
{
	int rc = 0;
	ldmsd_strgp_ref_t ref;

	if (strcmp(strgp->schema, prd_set->schema_name))
		return ENOENT;

	ref = strgp_ref_find(prd_set, strgp);
	switch (strgp->state) {
	case LDMSD_STRGP_STATE_STOPPED:
		if (ref) {
			LIST_REMOVE(ref, entry);
			ldmsd_strgp_put(ref->strgp);
			ref->strgp = NULL;
			free(ref);
		}
		break;
	case LDMSD_STRGP_STATE_RUNNING:
		rc = strgp_open(strgp, prd_set);
		if (rc)
			break;
		/* open success */
		strgp->state = LDMSD_STRGP_STATE_OPENED;
		/* let through */
	case LDMSD_STRGP_STATE_OPENED:
		rc = EEXIST;
		if (ref)
			break;
		rc = ENOMEM;
		ref = strgp_ref_new(strgp, prd_set);
		if (!ref)
			break;
		LIST_INSERT_HEAD(&prd_set->strgp_list, ref, entry);
		rc = 0;
		break;
	default:
		assert(0 == "Bad strgp state");
	}
	return rc;
}

/**
 * Given a producer set, check each storage policy to see if it
 * applies. If it does, add the storage policy to the producer set
 * and open the container if necessary.
 */
void ldmsd_strgp_update(ldmsd_prdcr_set_t prd_set)
{
	ldmsd_strgp_t strgp;
	int rc;
	ldmsd_cfg_lock(LDMSD_CFGOBJ_STRGP);
	for (strgp = ldmsd_strgp_first(); strgp; strgp = ldmsd_strgp_next(strgp)) {
		ldmsd_strgp_lock(strgp);
		ldmsd_name_match_t match = ldmsd_strgp_prdcr_first(strgp);
		for (rc = 0; match; match = ldmsd_strgp_prdcr_next(match)) {
			rc = regexec(&match->regex, prd_set->prdcr->obj.name, 0, NULL, 0);
			if (!rc)
				break;
		}
		if (rc) {
			ldmsd_strgp_unlock(strgp);
			continue;
		}
		ldmsd_strgp_update_prdcr_set(strgp, prd_set);
		ldmsd_strgp_unlock(strgp);
	}
	ldmsd_cfg_unlock(LDMSD_CFGOBJ_STRGP);
}

/* Caller must hold the strgp lock. */
int __ldmsd_strgp_start(ldmsd_strgp_t strgp, ldmsd_sec_ctxt_t ctxt)
{
	int rc;
	rc = ldmsd_cfgobj_access_check(&strgp->obj, 0222, ctxt);
	if (rc)
		return rc;
	if (strgp->state != LDMSD_STRGP_STATE_STOPPED)
		return EBUSY;
	strgp->state = LDMSD_STRGP_STATE_RUNNING;
	strgp->obj.perm |= LDMSD_PERM_DSTART;
	/* Update all the producers of our changed state */
	ldmsd_prdcr_update(strgp);
	return rc;
}

int ldmsd_strgp_start(const char *name, ldmsd_sec_ctxt_t ctxt, int flags)
{
	int rc = 0;
	ldmsd_strgp_t strgp = ldmsd_strgp_find(name);
	if (!strgp)
		return ENOENT;
	ldmsd_strgp_lock(strgp);
	if (flags & LDMSD_PERM_DSTART)
		strgp->obj.perm |= LDMSD_PERM_DSTART;
	else
		rc = __ldmsd_strgp_start(strgp, ctxt);
	ldmsd_strgp_unlock(strgp);
	ldmsd_strgp_put(strgp);
	return rc;
}

int __ldmsd_strgp_stop(ldmsd_strgp_t strgp, ldmsd_sec_ctxt_t ctxt)
{
	int rc = 0;

	ldmsd_strgp_lock(strgp);
	rc = ldmsd_cfgobj_access_check(&strgp->obj, 0222, ctxt);
	if (rc)
		goto out;
	if (strgp->state < LDMSD_STRGP_STATE_RUNNING) {
		rc = EBUSY;
		goto out;
	}
	if (strgp->state == LDMSD_STRGP_STATE_OPENED)
		strgp_close(strgp);
	strgp->state = LDMSD_STRGP_STATE_STOPPED;
	strgp->obj.perm &= ~LDMSD_PERM_DSTART;
	ldmsd_prdcr_update(strgp);
out:
	ldmsd_strgp_unlock(strgp);
	return rc;
}

int ldmsd_strgp_stop(const char *strgp_name, ldmsd_sec_ctxt_t ctxt)
{
	int rc = 0;
	ldmsd_strgp_t strgp = ldmsd_strgp_find(strgp_name);
	if (!strgp)
		return ENOENT;
	rc = __ldmsd_strgp_stop(strgp, ctxt);
	ldmsd_strgp_put(strgp);
	return rc;
}

extern struct rbt *cfgobj_trees[];
extern pthread_mutex_t *cfgobj_locks[];
ldmsd_cfgobj_t __cfgobj_find(const char *name, ldmsd_cfgobj_type_t type);

int ldmsd_strgp_del(const char *strgp_name, ldmsd_sec_ctxt_t ctxt)
{
	int rc = 0;
	ldmsd_strgp_t strgp;

	pthread_mutex_lock(cfgobj_locks[LDMSD_CFGOBJ_STRGP]);
	strgp = (ldmsd_strgp_t)__cfgobj_find(strgp_name, LDMSD_CFGOBJ_STRGP);
	if (!strgp) {
		rc = ENOENT;
		goto out_0;
	}

	ldmsd_strgp_lock(strgp);
	rc = ldmsd_cfgobj_access_check(&strgp->obj, 0222, ctxt);
	if (rc)
		goto out_1;
	if (strgp->state != LDMSD_STRGP_STATE_STOPPED) {
		rc = EBUSY;
		goto out_1;
	}
	if (ldmsd_cfgobj_refcount(&strgp->obj) > 2) {
		rc = EBUSY;
		goto out_1;
	}

	rbt_del(cfgobj_trees[LDMSD_CFGOBJ_STRGP], &strgp->obj.rbn);
	ldmsd_strgp_put(strgp); /* tree reference */

	/* let through */
out_1:
	ldmsd_strgp_unlock(strgp);
out_0:
	pthread_mutex_unlock(cfgobj_locks[LDMSD_CFGOBJ_STRGP]);
	if (strgp)
		ldmsd_strgp_put(strgp); /* `find` reference */
	return rc;
}

void ldmsd_strgp_close()
{
	ldmsd_strgp_t strgp = ldmsd_strgp_first();
	while (strgp) {
		ldmsd_strgp_lock(strgp);
		if (strgp->state == LDMSD_STRGP_STATE_OPENED)
			strgp_close(strgp);
		strgp->state = LDMSD_STRGP_STATE_STOPPED;
		ldmsd_strgp_unlock(strgp);
		/*
		 * ref_count shouldn't reach zero
		 * because the strgp isn't deleted yet.
		 */
		ldmsd_strgp_put(strgp);
		strgp = ldmsd_strgp_next(strgp);
	}
}
