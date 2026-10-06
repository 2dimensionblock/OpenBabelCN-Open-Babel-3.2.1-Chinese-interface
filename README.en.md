# OpenBabel Chinese Workbench

**A Chinese graphical application for 64-bit Windows, built on Open Babel 3.2.1 for chemical file conversion, molecular structure processing, and property calculation.**

[中文](README.md) · [Build guide](docs/BUILDING.md) · [Validation](docs/VALIDATION.md) · [Changelog](CHANGELOG.md)

The project combines a native C++ GUI, `OpenBabelCN.exe`, with a separate molecular computation engine, `obengine.exe`. The user interface is in Chinese; SMILES, format identifiers, force-field names, and raw upstream diagnostics retain their conventional forms. This is an independent application, not an official Open Babel GUI distribution.

> Application version: **1.0.0**. Windows binaries have been compiled, and 13 behavioral tests passed on a Linux engine built from the same source. Interactive testing on Windows hardware remains outstanding. The first public release should be marked as a pre-release pending that validation.

## Download and run

1. Open **Releases** on this repository and select a version.
2. Download **`OpenBabelCN_Windows64.zip`** and extract the entire archive.
3. Run **`OpenBabelCN.exe`**.
4. Add chemical files or enter SMILES, choose a task and output folder, and start processing.

Keep the adjacent `engine` directory. The portable package targets **Windows 10/11 x64** and requires no separate Python, Open Babel, or development-tool installation. Molecular computations run locally.

GitHub's automatic **Source code (zip)** download is a source snapshot. End users need the Windows package above. Developers can clone the repository or download the complete `OpenBabelCN_GitHub.zip` source package.

## Features

| Area | Capabilities |
| --- | --- |
| Input and batch jobs | Multiple files, drag-and-drop, one SMILES per line, batch output, molecule splitting |
| Format conversion | SDF, MOL, SMILES, PDB, PDBQT, MOL2, XYZ, InChI, SVG, and other registered formats |
| Coordinates | 2D and 3D coordinate generation with selectable 3D generation quality |
| Structure processing | Add/remove hydrogens, polar hydrogens, pH-based processing, largest connected fragment |
| Energy minimization | MMFF94, MMFF94s, UFF, GAFF, and Ghemical force fields |
| Partial charges | Gasteiger, MMFF94, and QEq |
| Docking-format preparation | Flexible-ligand and rigid-receptor PDBQT export |
| Properties | Formula, molecular weight, exact mass, H-bond counts, logP, TPSA, canonical SMILES, and more in Chinese-header CSV tables |
| Results | A 2D schematic of the first successful molecule, task summaries, and diagnostics |

Tasks can be cancelled. Results for each input are committed only after that input completes successfully. A failed or cancelled input does not publish partial structures as successful output; earlier successful results remain available.

## Example input

```text
CCO ethanol
CC(=O)Oc1ccccc1C(=O)O aspirin
Cn1c(=O)c2c(ncn2C)n(C)c1=O caffeine
```

Choose the molecular-properties preset to export a CSV table. For PDBQT output, supply a valid 3D structure or enable 3D coordinate generation first. Sample files are in [examples](examples).

## Scope and limitations

- PDBQT export prepares structures; the application does not perform docking searches, binding-affinity scoring, or missing-residue repair.
- The preview is schematic and does not render every stereochemical detail. SVG is available as an export format.
- Coordinate generation and minimization are limited to 500 heavy atoms per molecule. Previews are limited to 200 heavy atoms. Ordinary conversion and property calculation do not use that coordinate-processing limit.
- Each input task has a 10-minute timeout.
- This build disables CML/libxml2, JSON, PNG/Cairo, compressed-file I/O, Maestro, and external Coordgen support. Decompress compressed inputs before use.
- Force fields, charge models, and pH processing have applicability limits. Check resulting structures and parameters for the intended research task.

## Development

The application uses C++17 and Win32 Unicode APIs. The engine calls the bundled Open Babel C++ API directly. The portable distribution also includes the upstream `obabel.exe` command-line tool.

On Windows, install Python 3, CMake, Ninja, Perl, and a MinGW-w64 toolchain, then run:

```powershell
.\build-support\build_windows.ps1
```

See [BUILDING.md](docs/BUILDING.md) for dependencies, cross-compilation, testing, and CI scope. The bundled [test results](tests/test_results.json) are historical results, not a record of a GitHub Actions run. The added workflow must be run in the destination repository to establish its actual status.

## Feedback and licensing

Please use repository **Issues** for bug reports and feature requests. Include the application version, Windows version, reproduction steps, and a minimal input that can be shared publicly. See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidance.

New application code is licensed under **GPL-2.0-only**; see [LICENSE](LICENSE). Third-party files retain their original copyrights and licenses. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). Distribute the corresponding source and license notices alongside the binaries.

The chemistry functionality is provided by [Open Babel](https://openbabel.org/). Thanks to the contributors to Open Babel, InChI, Eigen, MinGW-w64, and related projects. The supplied source archive, documented patches, and build scripts are included in this repository.
