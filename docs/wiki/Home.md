# Momentum Wiki

Momentum is a dynamic-derivative compressor: it reacts to how fast the loudness is changing, not only how loud the signal is.
Two RMS envelope followers run in parallel on the mono sum of the input; one has a window set by the Speed control and the
other is fixed at 300 ms.

- [User Guide](User-Guide.md)
- [Developer Guide](Developer-Guide.md)
- [Codebase Map](Codebase-Map.md)

Authoritative: `Momentum/README.md` (controls, build, interface, deviations from the spec) and `ELI5.md`.
