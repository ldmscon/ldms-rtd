.. _ldmscon-overview:

LDMSCON2025
============

**FIX ME**: Modify the following section below by updating "align", "width" and "height" sections to "center", "300", and "300" respectively.

.. image:: ../images/LDMSCON2025.png
   :alt: OVIS-HPC Logo
   :align: left
   :width: 950
   :height: 75

Overview
--------

**LDMS (Lightweight Distributed Metric Service)** is a scalable monitoring infrastructure for high-performance computing (HPC) environments. It collects, transports, stores, and queries performance and resource usage metrics.

**LDMSCON** is a tool that allows users to interactively inspect and configure `ldmsd` daemons useful for validation and debugging during development.

For more detailed reference material and additional resources, please see the :ref:`useful_links` section below.

Key Concepts
^^^^^^^^^^^^

- **ldmsd**: The LDMS daemon that loads plugins and collects metrics.
- **LDMSCON**: A console tool to connect to a running ``ldmsd`` instance and interact with it.
- **plugins**: Modular metric collectors that can be configured in ``ldmsd`` (e.g., ``meminfo``).
- **transports**: Mechanisms for communication, such as `sock`, `rdma`, or `ugni`.

**FIX ME**: make a reference to the example meminfo manpage (e.g. :ref:`description <label>`). This will be configured in a later exercise.   

* Details on how to configure the meminfo sampler can be found at the  meminfo example man page <meminfo-ex>

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

All subproject documentation is integrated and accessible through the main site:

**FIX ME**: Update the following to correctly reference the document of the parent project that was cross-referenced in conf.py (e.g. :doc:`description <parent-project:rst-file>`)

- OVIS-HPC Documentation Site <:>

.. _useful_links

Useful Links
------------

- :doc:`OVIS-HPC subprojects using intersphinx_mapping <ovis-hpc:projects>`
- :ref:`Quick Start using :ref: <ldms-quick-start>`
- :doc:`Quick Start using :doc: <../intro/quick-start>`
- `LDMS Source Code using URL <https://github.com/ovis-hpc/ldms>`_

