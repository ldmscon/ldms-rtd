/* -*- c-basic-offset: 8 -*-
 * Copyright (c) 2019 National Technology & Engineering Solutions
 * of Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with
 * NTESS, the U.S. Government retains certain rights in this software.
 * Copyright (c) 2019 Open Grid Computing, Inc. All rights reserved.
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
/**
 * \file tutorial_sampler.c
 * \brief tutorial data provider
 */
#define _GNU_SOURCE
#include <inttypes.h>
#include <unistd.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include "ldms.h"
#include "ldmsd.h"
#include "sampler_base.h"


#define MAXSETS 5
#define DEFAULTNUMMETRICS 10
#define SAMP "tutorial_sampler"

struct tutorial_set {
	ldms_schema_t schema;
	base_data_t base;
	ldms_set_t set;
	int num_metrics;
	int metric_offset;
};

static struct tutorial_set tsets[MAXSETS];
static ldmsd_msg_log_f msglog;
static int num_sets = 0;


static int create_metric_set(struct tutorial_set* tset)
{
	int rc, i, j;
	ldms_schema_t schema;
	ldms_set_t set;
	uint64_t metric_value;
	char metric_name[128];
	const char *s;

	msglog(LDMSD_LINFO, SAMP ": Calling create_metric_set\n");

	schema = base_schema_new(tset->base);
	if (!schema) {
		msglog(LDMSD_LERROR,
		       "%s: The schema '%s' could not be created, errno=%d.\n",
		       __FILE__, tset->base->schema_name, errno);
		rc = errno;
		goto err;
	}

	/* Location of first metric from proc/tutorial_sampler file */
	tset->metric_offset = ldms_schema_metric_count_get(schema);

	// add metrics
	for (i = 0; i < tset->num_metrics; i++){
		rc = snprintf(metric_name, 128, "%s%d", "metric", i);
		msglog(LDMSD_LINFO, SAMP ": adding metric '%s'\n", metric_name);
		rc = ldms_schema_metric_add(schema, metric_name, LDMS_V_U64);
		if (rc < 0) {
			rc = ENOMEM;
			goto err;
		}
	};

	msglog(LDMSD_LINFO, SAMP ": About to call base_set_new schema\n");
	set = base_set_new(tset->base);
	if (!set) {
		msglog(LDMSD_LDEBUG, SAMP ": base_set_new failing\n");
		rc = errno;
		goto err;
	}
	tset->schema = schema;
	tset->set = set;


	// TUT: Comment in/out -- what are our full variables? From the set
	msglog(LDMSD_LINFO, SAMP ": full metrics: (%d in base)\n", tset->metric_offset);
	j = ldms_set_card_get(set);
	for (i = 0; i < j; i++){
		s = ldms_metric_name_get(set, i);
		msglog(LDMSD_LINFO, SAMP ": full metric %d '%s'\n", i, s);
	}


	msglog(LDMSD_LINFO, SAMP ": Leaving create_metric_set\n");
	return 0;

 err:
	msglog(LDMSD_LDEBUG, SAMP ": exiting create_metric_set with error %d\n", rc);
	return rc;
}


static const char *usage(struct ldmsd_plugin *self)
{
	return  "config name=" BASE_CONFIG_USAGE SAMP " schema=<schemaname> num_metrics=<N>\n";
}

static int config(struct ldmsd_plugin *self, struct attr_value_list *kwl, struct attr_value_list *avl)
{
	char *value;
	int rc;

	// TUT: demo of log lines in file
	msglog(LDMSD_LINFO, SAMP ": Calling config for set %d\n", num_sets);

	if (num_sets == (MAXSETS-1)){
		msglog(LDMSD_LERROR, SAMP ": Too many sets.\n");
		return EINVAL;
	}

	tsets[num_sets].num_metrics = DEFAULTNUMMETRICS ;
	value = av_value(avl, "num_metrics");
	// TUT: first run with negative value, will get zero
	// TUT: then run with BAD check, value is char*
        if (value && (atoi(value) > 0)){
	//        if (value && (value > 0)){ # BAD
                tsets[num_sets].num_metrics = (uint64_t)(atoi(value));
	}

	//producer, component_id, instance, schema etc all in base_config
	tsets[num_sets].base = base_config(avl, SAMP, SAMP, msglog);
	if (!tsets[num_sets].base) {
		rc = errno;
		goto err;
	}

	rc = create_metric_set(&(tsets[num_sets]));
	if (rc) {
		msglog(LDMSD_LERROR, SAMP ": failed to create a metric set.\n");
		goto err;
	}

	num_sets++;

	msglog(LDMSD_LINFO, SAMP ": Leaving config\n");
	return 0;


 err:
	base_del(tsets[num_sets].base);
	return rc;
}

static ldms_set_t get_set(struct ldmsd_sampler *self)
{
	return NULL;
}

static int sample(struct ldmsd_sampler *self)
{
	int rc;
	int metric_no;
	char *s;
	char lbuf[256];
	char metric_name[128];
	int i,j;
	union ldms_value v;


	for (i = 0; i < num_sets; i++){
		msglog(LDMSD_LINFO, SAMP ": sampling for set %d\n", i);
		
		if (!tsets[i].set) {
			msglog(LDMSD_LDEBUG, SAMP ": plugin not initialized\n");
			return EINVAL;
		}

		//TUT: comment out/in base_sample - won't get timestamp, always inconsistent
		base_sample_begin(tsets[i].base);

		for (j = 0; j < tsets[i].num_metrics; j++){
			metric_no = tsets[i].metric_offset + j;
			v.v_u64 = ldms_metric_get_u64(tsets[i].set, metric_no);
			v.v_u64+= (i+1)*(j+1);
			ldms_metric_set(tsets[i].set, metric_no, &v);
		}

		//TUT: comment out/in base_sample
		base_sample_end(tsets[i].base);
	}
 out:

	return 0;
}

static void term(struct ldmsd_plugin *self)
{

	int i;
	
	for (i = 0; i < num_sets; i++){
		if (tsets[i].schema)
			ldms_schema_delete(tsets[i].schema);
		tsets[i].schema = NULL;

		if (tsets[i].base)
			base_del(tsets[i].base);
		tsets[i].base = NULL;

		if (tsets[i].set)
			ldms_set_delete(tsets[i].set);;
		tsets[i].set = NULL;
		tsets[i].num_metrics = 0;
		tsets[i].metric_offset = 0;
	}
	num_sets = 0;
}

static struct ldmsd_sampler tutorial_sampler_plugin = {
	.base = {
		.name = SAMP,
		.type = LDMSD_PLUGIN_SAMPLER,
		.term = term,
		.config = config,
		.usage = usage,
	},
	.get_set = get_set,
	.sample = sample,
};

struct ldmsd_plugin *get_plugin(ldmsd_msg_log_f pf)
{
	msglog = pf;
	return &tutorial_sampler_plugin.base;
}
