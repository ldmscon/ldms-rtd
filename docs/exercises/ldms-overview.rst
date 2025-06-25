.. ldmscon-overview:

LDMSCON2025
============

.. image:: ../images/LDMSCON2025.png
   :alt: LDMSCON2025 Logo
   :align: center
   :width: 300
   :height: 300

Overview
--------

**LDMS (Lightweight Distributed Metric Service)** is a scalable monitoring infrastructure for high-performance computing (HPC) environments. It collects, transports, stores, and queries performance and resource usage metrics.

**LDMSCON** is a tool that allows users to interactively inspect and configure `ldmsd` daemons useful for validation and debugging during development.

For more detailed reference material and additional resources, please see the `Useful Links`_ section below.

Key Concepts
^^^^^^^^^^^^

- **ldmsd**: The LDMS daemon that loads plugins and collects metrics.
- **LDMSCON**: A console tool to connect to a running ``ldmsd`` instance and interact with it.
- **plugins**: Modular metric collectors that can be configured in ``ldmsd`` (e.g., ``meminfo``).
- **transports**: Mechanisms for communication, such as `sock`, `rdma`, or `ugni`.

* Details on how to configure the meminfo sampler can be found at the :ref:`meminfo example man page <meminfo-ex>`

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

**FIX ME**: Utilize Sphinx's `intersphinx_mapping`, defined in conf.py, to reference the parent project homepage (e.g. :doc:`description <parent-project:rst-file>`)

- :doc`OVIS-HPC Documentation Site <ovis-hpc:index>`


Useful Links
------------

- :doc:`OVIS-HPC subprojects using intersphinx_mapping <ovis-hpc:projects>`
- :ref:`Quick Start using :ref: <ldms-quick-start>`
- :doc:`Quick Start using :doc: <../intro/quick-start>`
- `LDMS Source Code using URL <https://github.com/ovis-hpc/ldms>`_

