**FIX ME**: Add a label called "meminfo-ex"
 
**FIX ME**: Add the appropriate underline styles: ==== above and below the title, and ----- above and below the description.
Note: 

        All underline styles must be at least as long as the title/description to avoid Sphinx build warnings..


meminfo-ex


Example man page for the LDMS meminfo plugin


**FIX ME**: Add the date (todays date), section (7) and group (LDMS samlper) with appropriate syntax (e.g. :Date: day month year, :Manual section:, :Manual group:).
:
:
:

SYNOPSIS
========

| Within ldmsd_controller or a configuration file:
| config name=meminfo [ <attr>=<value> ]

DESCRIPTION
===========

With LDMS (Lightweight Distributed Metric Service), plugins for the
ldmsd (ldms daemon) are configured via ldmsd_controller or a
configuration file. The meminfo plugin provides memory info from
/proc/meminfo.

CONFIGURATION ATTRIBUTE SYNTAX
==============================

The meminfo plugin uses the sampler_base base class. This man page
covers only the configuration attributes, or those with default values,
specific to the this plugin; see :ref:`ldms_sampler_base(7) <ldms_sampler_base>` for the
attributes of the base class.

**config**
   | name=<plugin_name> [schema=<sname>]
   | configuration line

   name=<plugin_name>
      |
      | This MUST be meminfo.

   schema=<schema>
      |
      | Optional schema name. It is intended that the same sampler on
        different nodes with different metrics have a different schema.
        If not specified, will default to \`meminfo`.

BUGS
====

No known bugs.

EXAMPLES
========

Within ldmsd_controller or a configuration file:

**FIX ME**: Format the following as a code block using either :: or .. code-block::. Ensure there is a blank line after the directive, and indent the code properly.

load name=meminfo
config name=meminfo producer=vm1_1 instance=vm1_1/meminfo
start name=meminfo interval=1000000

SEE ALSO
========

**FIX ME**: Fix the syntax of the ldmsd and ldms_quickstart references

ref:`ldmsd(8) ldmsd`, :ref:ldms_quickstart(7) <ldms_quickstart>, :ref:`ldmsd_controller(8) <ldmsd_controller>`, :ref:`ldms_sampler_base(7) <ldms_sampler_base>`
