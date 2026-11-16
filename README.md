# Momentum

Momentum is a dynamic-derivative compressor project. The application source and its project documentation are in the Momentum/ directory.

- [ELI5: What Momentum does](ELI5.md)
- [Project and build guide](Momentum/README.md)
- [Progress notes](PROGRESS.md)
- [Wiki source](docs/wiki/Home.md): user guide, developer guide, design rationale and codebase map



## Required shared plug-in standard

This project follows the [Circuit Drift Labs Shared Audio Plugin Standard](docs/standards/CDL_PLUGIN_BASELINE.md). It is required for the plug-in target; standalone-only requirements apply only when a standalone target is included. The product-specific specification supplements the shared standard and records the applicable profiles, compliance status, and any exceptions.

## Repository hygiene (noted, not changed)

`Momentum/build-lite/` (about 256 files, including `CMakeCache.txt`) is committed to the repository. Build output normally belongs in `.gitignore`. Consider removing it from the index.
