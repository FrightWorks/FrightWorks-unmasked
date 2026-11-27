# FrightWorks: UnMasked: shared notes for agents

Rules and facts every agent working on this repo needs, whichever tool it runs in.
This file is shared with everyone who can read the repo.
People and agents also keep their own git-ignored `AGENTS.md` at the repo root for anything specific to their machine (see "Your local AGENTS.md").
The rules follow the sister project, FrightWorks-no100f (its `.agents/REPO.md`), adapted to what is true here.

## Using this file

- **Read it before changing anything.**
  Skills, when there are some, live in `.agents/skills/`: read the matching one first.
  Claude Code looks for skills in `.claude/skills/` (git-ignored): run `python tools/setup-agents.py` once per checkout to link it to `.agents/skills/`.
- **Change it in the PR that makes the change true.**
  A new rule or fact ships in the same diff as the work behind it.
  Standalone corrections are fine as their own small PR.
- **Rules and lasting facts only.**
  Who is working on what belongs in GitHub issues and PRs.
  Case histories belong in skills.
- **Write it as if it were public, and remember that agents act on it.**
  No personal paths, machine details, credentials, logs or game data.
  Review edits to `.agents/` as carefully as code.
- **One sentence per line.**
  Two people editing different sentences of one paragraph then don't conflict.

## Your local AGENTS.md

Keep a root `AGENTS.md` for your own setup.
The repo's `.gitignore` already lists it and `CLAUDE.md`: never commit either.
Put a line in it pointing here, and fill in:

| Item | Value |
| --- | --- |
| You | Your GitHub name. |
| Git identity | The name and email your commits use. Check it before committing. |
| GitHub CLI | The `gh` login you use for this repo. `gh auth setup-git` makes git use it for github.com. |
| Checkout | Where your clone lives. |
| Remotes | `origin` = https://github.com/FrightWorks/FrightWorks-unmasked.git. |
| OS and GPU | Your Windows version and graphics card (it matters for MSAA and test results). |
| Game | Your `Play/` is your own game and settings. Tests don't run from it (see Testing). |

## The project

PC port of Scooby-Doo! Unmasked (GameCube, USA, G5DE78): a static recompilation built on WiiCompiled + Aurora.
GPLv3 (from WiiCompiled), which can't be relicensed.
Based on Scooby-Doo: Re-Masked by JosiahG (v1.0.1); the same author's Refrightened is where FrightWorks-no100f started, so the two share most of `wiicompiled/`.
Windows x64 only for now.

| Path | What |
| --- | --- |
| `wiicompiled/translator` | C#: the game's PowerPC code to C++ |
| `wiicompiled/runtime` | Native runtime, GameCube SDK HLE and game overrides (C++) |
| `wiicompiled/aurora-main` | Graphics, window, input (SDL3, WebGPU through Dawn) |
| `wiicompiled/projects/scooby` | This game's project: symbols, function map, address remaps, logic-module tools |
| `launcher/` | Launcher (C# WinForms, .NET 8, Windows only) |
| `tools/` | Build, package and test scripts; `check-assets.py` (before every PR), `setup-agents.py` (skills link) |
| `Disc/`, `game/`, `Play/`, `dist/`, `toolchain/`, `autotest/`, `build-logs/` | Git-ignored: disc files, build output, downloaded tools, work data |

Some names still say "Re-Masked" (the launcher project, the exe names, the build and package scripts, `package-README.txt`).
Renaming them is its own PR: the exe names are wired through the scripts, the launcher and the runtime cmake.

## Rules

**Git and pull requests**
- Maintainers push topic branches to `origin` and open the PR from them.
  Contributors without write access work in their own fork.
- Everything reaches `main` through a PR, small docs changes too.
  Never push to `main`.
  Never force-push.
  GitHub can't enforce this here (rulesets need a public repo or a paid plan), so it holds by agreement: the same rule as no100f, where rulesets do enforce it.
- After a rebase or any history rewrite, check that the branch's diff against `origin/main` contains only your own changes.
  Resetting onto a `main` that moved, while your files still match the old one, silently undoes the new commits.
- Fix-only PRs branch off `main` with minimal diffs and no heuristics that hide symptoms.
  Diagnostics don't go in a fix PR.
- Before opening a PR, run `python tools/check-assets.py`: it fails if the branch or the staged files add anything under "Never commit" below (by path, file type, size and header).
  It is run by hand, not a hook.
- Open PRs as drafts.
  Say plainly what was and wasn't tested.
- Write "fixes #N" or "closes #N" in a PR only when merging it should close that issue: GitHub matches the words even in "doesn't fix #N".
- PR bodies mention other PRs only if they were tested together, with conflict status.
  No tool-generated footer, and no tool attribution in commits (no `Co-Authored-By` trailer for an AI tool).
- Only @-mention someone who needs to act.
- There is no CI: never check or report CI status.
- Before editing a PR or issue body, fetch the current one and edit that: someone may have added media.
- `gh` cannot upload attachments.
  Screenshots: no personal paths, names or addresses on screen, screenshots only (never disc or game files), and agents look at each picture before it goes up.
  Readable at GitHub's display size: zoomed crops or a few large panels, not grids of small screenshots.
- Keep contributors as commit authors when moving their work.
- Issue labels: at most one per category (`type:`, `area:`, `priority:`, `status:`).

**Never commit**
- A disc image, `Disc/`, `game/`, or anything extracted or dumped from the game: `main.dol`, `main_logic.dol`, the logic modules, textures, sounds, movies.
- The build's disc-derived files: `wiicompiled/projects/scooby/MAP_all.txt` and `MAP_logic.txt` (`.gitignore` covers both).
- Any picture made from the game's textures (the button-prompt art in `tools/button-art/` is Kenney's, not the game's).
- Saves, local builds, `Play/` contents beyond `portable.txt` and `Run with log.cmd`.
- Root `AGENTS.md` or `CLAUDE.md`, logs, recordings, or personal paths.
- One-off scripts and their output: they stay in the git-ignored `autotest/` folder.

**Docs**
- README is for players and builders only.
- Implementation notes go in code comments and PR bodies; lasting facts agents need go here.
- Player-visible fixes go in release notes.
- A new F10 video setting with a lasting effect belongs on the launcher's Display page too.

**Licences of bundled art:** the launcher's title font (`launcher/title.ttf`, "Scooby Doo" by Lauren Ashpole) is free for personal use only, and the Mystery Machine and Scooby pictures came with the upstream port.
Check the licence of anything added to the launcher or the release, and keep the credits that a licence asks for.

**No bypassing security warnings:** do not bypass antivirus or OS warnings, add exclusions or restore quarantined files.

**No telemetry anywhere:** builds, tools, launcher, game, dependencies.
Check new dependencies for it.
Set `DOTNET_CLI_TELEMETRY_OPTOUT=1` when running `dotnet` by hand (PowerShell: `$env:DOTNET_CLI_TELEMETRY_OPTOUT = '1'; dotnet ...`); `build-launcher.ps1` sets it itself.

**New downloads:** when adding a download to the build or tools, or changing its source, check what it fetches and from where.
Pin a version and hash where the source allows it, and say so in the PR.
A new download that runs an installer or changes security settings needs a maintainer's OK.

**Credits:** maintainers BitsofBitsofBits and LoneRanger419; original port by JosiahG; WiiCompiled by patchzyy and contributors.
They are in the README; the launcher header still says "Made by JosiahG" and `tools/package-README.txt` still has the upstream credit lines.
A credit change goes in all three.

## Testing

Keep game launches short and critical.
Test code without launching the game where that can work.

| Change | Run |
| --- | --- |
| Docs | Nothing |
| Launcher | `--selftest` (builds every page, exit 0) and `--shots <dir>` (every page to PNG); a person checks it only if it looks different |
| Disc extraction (`launcher/GcDisc.cs`) | `--extract <image> <outdir>` on a real disc image (exit 0 ok, 2 holes, 3 wrong disc); the output is disc data, keep it local |
| Translator | Translator tests (`wiicompiled/translator/tests/Translator.Tests`, xUnit) + a short game run |
| Game runtime or overrides | Runtime tests (CTest, `wiicompiled/runtime/tests`) + a short game run; a person checks it only if behaviour changes |
| Graphics (Aurora) | A short game run + a person checks it |
| Audio | A short game run + a person listens |

There is no smoke test, test kit or dev menu here yet (no100f has them); anything beyond the table needs a stated reason.

**Habits**
- A build passing, tests passing and a real game run are different evidence.
  Say which one you have, and do not claim more.
- A test program that is missing, not built, not registered with CTest, or that could not load is not a pass.
  Report it as not run.
- Show a bug on unmodified `main` first, then show it gone with your change.
- Renderer changes must preserve colour space, transparency, depth and capture behaviour.
- Wait on log lines or files, not fixed sleeps, where you can.
- Close only the test game, and check afterwards that only it closed.

**Test runs**
- Never test from your own `Play/`, which holds your game and settings.
  Use a worktree (it has its own `Play/`) or a separate test folder with a copy of the freshly built game and its own `UserData/Config.toml` (windowed, muted, rumble off).
  `tools/level-sweep.py` runs each window from its own copy of `Play/`; `tools/level-sweep.ps1` runs from `Play/` itself and adds a mod to its `UserData`, so don't use it on a `Play/` that someone plays from.
- Give automated game runs a time limit.
- Leave the user's setup as found: put back any `Config.toml` or controller setting a test changed.
- Rumble off (`[controller] rumble = false`), always.
  The game's default is on, so check before any launch.
- Sound muted (`[audio] muted = true`), unless a person is testing or playing it or the task is audio: then on at 5% (`[audio] volume = 0.05`).
- Don't use the level sweep or several windows for visual bugs: one window, looked at by a person.
- Check that nobody's game is running before a run on a shared desktop: a test game can act on a pad someone is using.

## Building

```powershell
powershell -ExecutionPolicy Bypass -File tools\build-launcher.ps1      # launcher\publish\Scooby-Doo Re-Masked Launcher.exe
# Extract your disc to Disc\DATA (the translation reads the game's code from it):
& "launcher\publish\Scooby-Doo Re-Masked Launcher.exe" --extract "C:\path\to\game.iso" Disc\DATA
powershell -ExecutionPolicy Bypass -File tools\build-scooby.ps1        # full build -> Play\
powershell -ExecutionPolicy Bypass -File tools\package-scooby.ps1 -Zip # dist\ScoobyDooReMasked\ and the zip
```

- The first build downloads LLVM-MinGW, CMake, Ninja, SDL3, Dawn and a portable .NET 8 SDK into `toolchain\` and `wiicompiled\Dependencies\`; nothing is installed system-wide.
- `build-scooby.ps1` options: `-SkipTools`, `-SkipTranslate`, `-ConfigureOnly`, `-Jobs N`, `-LogicLevels` (default `all`).
  `-LogicLevels MAINMENU` or a list such as `MAINMENU,W1L1` translates only those levels' logic modules: a faster build that can't play the other levels.
- A build needs the extracted `Disc\DATA`, not the ISO.
- Build logs go to `build-logs\`.

## Facts worth knowing

**Game code**
- The disc ships an unstripped release ELF whose sections match `main.dol`, so all 6221 functions and 15887 data symbols have real names (`wiicompiled/projects/scooby/elf_symbols.txt`, `elf_data_symbols.txt`, `MAP.txt`).
  Disassembler: `python wiicompiled/projects/scooby/tools/scooby_dis.py <symbol|addr> [count]` (needs capstone).
- The levels' game logic is compiled PowerPC in relocatable ELF objects inside each `.ghr` (about 1557 unique modules), which the game loads and relocates at run time (`EFLogicCode::LoadFile`, `0x800E799C`).
  `projects/scooby/tools/build_logic_overlay.py` (run by the build) places every unique module at a fixed address in an overlay above MEM1 (`0x81800000`..`0x83000000`), relocates it like the game, and writes `game/main_logic.dol`, `MAP_logic.txt` and `runtime/src/hle/scooby_logic_table.inc`.
  `runtime/src/hle/scooby_logic.cpp` hashes each module the game loads and hands back its fixed address, so it runs the translated code; `[logic] UNKNOWN` in the log means a module the build never saw (it would run untranslated).
- No game code or data is in the exe: `runtime/src/hle/scooby_dol_loader.cpp` rebuilds `main_logic.dol` from the player's disc at startup (a C++ mirror of `build_logic_overlay.py`: change both together) and checks it against the SHA-256 in `hle/scooby_logic_dol_sha256.inc`, which `build-scooby.ps1` writes.
- `generated\build_shards\base_common_shard_map.json` freezes shard boundaries: after the function set grows a lot (a `-LogicLevels` change), delete it, or every new function lands in one giant shard.
- Adding or removing overrides, or switching between branches that differ in them, needs a full translate (no `-SkipTranslate`).

**GameCube HLE**
- GameCube FST offsets are bytes (the Wii's are words shifted by 2): `dvd.cpp` converts.
- Memory card: the 19 CARD calls go to Aurora's GCI-folder card in `UserData\Saves\MemoryCard` (`hle/storage/card_gc.cpp`); `[card]` log lines show every result.
- ARAM is 16 MB on the host (`hle/os/gc_hw.cpp`); voice-line streams depend on `ARQPostRequest_Gc` filling the request's owner.
- GameCube AX audio is ported from Dolphin (`ax_mix.cpp` `ExecuteGcCommandList`, selected by `kGameCubeAx`), mixed on the guest thread at the game's 32 kHz.
  A 48 kHz render was tried and removed: keep 32 kHz.
- Wii-only paths (NAND, IOS, WPAD) are still compiled in with their hooks disabled.

**Launcher and packaging**
- The launcher walks up to the folder holding `Play\Scooby-Doo Re-Masked.exe` (dev tree) or `data\` (release), so in the dev tree extracting a disc replaces the dev `Disc\DATA`.
- `package-scooby.ps1` updates `dist\ScoobyDooReMasked` in place: it replaces program files only and keeps `data\Disc` and `data\UserData`, because a person may play from `dist`.
  `-Zip` stages a clean copy in a temp folder.
- Portable layout (`data\portable.txt`): settings, saves, logs and mods in `data\UserData`, disc in `data\Disc\DATA`.

**Diagnostics** (environment variables and log tags left in from the port)
- `SCOOBY_TRACE_IO=1` (`[io]` completions, `[aram]` transfers), `[band]`, `[gpuvp]`, `[vpmap]`, `[mode]` and `[card]` lines (each capped per run).
- `RUNTIME_NO_POPUP=1` logs a crash instead of showing a popup; stdout and stderr carry the log, so redirect them.

## Releases

A maintainer runs releases and publishes them.

1. Version PR: `<Version>` in `launcher/ScoobyReMasked.Launcher.csproj` and the version in `tools/package-README.txt` if it names one.
2. After the merge, on a clean `main`: `tools/package-scooby.ps1 -Build -Zip`.
3. Check the zip has no game data, saves or settings.
4. `gh release create v<ver> --draft --target <full SHA>` with the zip and the notes.
   A maintainer publishes.
