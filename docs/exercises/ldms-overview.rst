**FIX ME**: Add the appropriate syntax to the following label (ex: ".. _ldmscon-overview_spwalto:")
ldmscon-overview_<username>

LDMSCON & LDMS Overview
=======================

**FIX ME**: Modify the following section below by updating "align" and "width" sections to "center" and "40%", respectively.

.. image:: ../images/main-logo.png
   :alt: LDMS Logo
   :align: left
   :width: 15%
   :height: 95%

Overview
--------

**LDMS (Lightweight Distributed Metric Service)** is a scalable monitoring infrastructure for high-performance computing (HPC) environments. It collects, transports, stores, and queries performance and resource usage metrics.

**LDMSCON** is a tool that allows users to interactively inspect and configure `ldmsd` daemons useful for validation and debugging during development.

For more detailed reference material and additional resources, please see the :ref:`useful_links` section below.

Key Concepts
^^^^^^^^^^^^

- **ldmsd**: The LDMS daemon that loads plugins and collects metrics.
- **LDMSCON**: A console tool to connect to a running ``ldmsd`` instance and interact with it.
- **Plugins**: Modular metric collectors that can be configured in `ldmsd` (e.g., `meminfo`).
- **Transports**: Mechanisms for communication, such as `sock`, `rdma`, or `ugni`.

**FIX ME**: Replace <username> with your email username in the reference below. This label will be created in a later exercise 

* Details on how to configure the meminfo sampler can be found at the :ref:`meminfo example man page <meminfo-ex_<username>>`

Documentation
^^^^^^^^^^^^^

OVIS-HPC leverages Read The Docs to provide centralized and structured documentation across all of its subprojects. This includes:

- High-level overviews and architectural descriptions,
- Step-by-step installation and configuration guides,
- Usage examples and walkthroughs,
- Reference material for APIs, command-line tools, and configuration options,
- Developer guides for contributing to the codebase or building custom plugins,
- Troubleshooting tips and FAQs.

The documentation is organized by project and version, and supports cross-referencing between the main project (OVIS-HPC) and its subcomponents using Sphinxs `intersphinx_mapping`.

**FIX ME**: Update the following to correctly reference the document of the parent project that was cross-referenced in the `intersphinx_mapping` in conf.py: ``:doc:`display name <parent-project:rst-file>``

All subproject documentation is integrated and accessible through the main site:

- doc:OVIS-HPC Documentation Site <ovis-hpc:index>

.. _useful_links

Useful Links
------------

- :doc:`OVIS-HPC subprojects using intersphinx_mapping <ovis-hpc:projects>`
- :ref:`Quick Start using reference <ldms-quick-start>`_
- :doc:`Quick Start using document <../intro/quick-start>`
- `LDMS Source Code using URL <https://github.com/ovis-hpc/ldms>`_

