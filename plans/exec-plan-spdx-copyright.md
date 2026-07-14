# Standardize SPDX copyright notices and maintainer attribution

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

This plan must be maintained in accordance with the repository's `PLANS.md`. If the repository does not yet contain `PLANS.md`, follow the structure and requirements documented by OpenAI for Codex ExecPlans.

## Purpose / Big Picture

The repository currently identifies the company in source-file copyright notices, but the notices may not be uniform, machine-readable, or easy to validate. The project is maintained by Fabio Sobral, whose role should also be visible to contributors and users without changing the legal copyright holder.

After this change:

1. applicable source files use standardized SPDX headers;
2. the copyright holder is written exactly as:

       SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.

3. the license identifier is:

       SPDX-License-Identifier: LGPL-2.1-only

4. Fabio Sobral is visibly identified as the project creator and lead maintainer in repository-level documentation;
5. `AUTHORS.md` records project authorship and maintenance responsibility;
6. `CODEOWNERS` identifies `@flsobral` as the default code owner;
7. an automated validation command detects missing, malformed, or obsolete copyright headers;
8. continuous integration runs the validation for pull requests and relevant branch updates;
9. contributor documentation explains how copyright headers must be added to new files.

The implementation must distinguish legal ownership from project attribution:

- `Amalgam Solucoes em TI Ltda.` is the copyright holder.
- `Fabio Sobral` is the creator and lead maintainer.
- Fabio Sobral must not be added as an additional copyright holder unless a separate legal decision explicitly requires it.

A contributor must be able to clone the repository, run one documented validation command, and receive a clear success or failure result.

## Progress

- [x] (2026-07-14) Inspect repository structure, existing copyright notices, licensing files, documentation, CI workflows, and repository-specific instructions.
- [x] (2026-07-14) Confirm the repository had no authoritative license text; add the LGPL 2.1 text required by this plan.
- [x] (2026-07-14) Define canonical C-style, hash-comment, XML, and HTML SPDX headers and classify 59 historical first-party files.
- [x] (2026-07-14) Implement `scripts/check_copyright.py`, a deterministic validator using `git ls-files -z`.
- [x] (2026-07-14) Add 12 dependency-free fixture tests in `scripts/test_check_copyright.py`.
- [x] (2026-07-14) Migrate applicable tracked source files to the canonical SPDX format throughout rewritten history.
- [x] (2026-07-14) Create `refs/backup/pre-spdx-main` and locally rewrite the 29-commit linear history on `spdx-history` in a disposable clone.
- [x] (2026-07-14) Add `LICENSE`, `AUTHORS.md`, `.github/CODEOWNERS`, and maintainer attribution to the rewritten initial commit.
- [x] (2026-07-14) Ensure canonical SPDX headers exist from each applicable file's introduction commit onward.
- [x] (2026-07-14) Validate commit metadata and topology; record the old-to-new mapping in `docs/history/spdx-copyright-rewrite-2026-07-14.md`.
- [x] (2026-07-14) Document contributor rules in `CONTRIBUTING.md` and maintainer attribution in `README.md`.
- [x] (2026-07-14) Add `.github/workflows/copyright.yml` for pull requests and pushes.
- [x] (2026-07-14) Run validator tests, current-tree validation, historical introduction validation, metadata/topology checks, and the desktop CMake build.
- [x] (2026-07-14) Review the final diff and record results in `Outcomes & Retrospective`.

## Surprises & Discoveries

Record repository-specific findings here as implementation proceeds.

Examples of findings that belong in this section:

- different historical company names in existing headers;
- files licensed under terms different from the main repository;
- vendored dependencies containing third-party copyright notices;
- generated files that must not be edited manually;
- scripts or build tools that already perform partial license validation;
- source formats requiring unusual comment syntax;
- files whose first line must remain a shebang, XML declaration, encoding declaration, or another interpreter directive.

Do not silently normalize exceptions. Document each important exception and the reason for it.

- Observation: The original repository had no `LICENSE`, `AUTHORS.md`, or root `PLANS.md`.
  Evidence: `git ls-tree -r 4136340` contained none of these files; `.agent/PLANS.md`
  supplies the applicable ExecPlan maintenance guidance.

- Observation: The only pre-existing copyright and license markers are Apache-2.0
  notices in the Gradle wrapper.
  Evidence: `git grep` found notices only in `android/gradlew` and
  `android/gradlew.bat`; the validator excludes `android/gradle/wrapper/` and
  these third-party wrapper files are unchanged.

- Observation: A `git filter-branch` tree filter receives each original commit
  tree, so applying headers only in the introducing commit loses them when a
  later original commit modifies that file.
  Evidence: the first disposable rewrite passed at its root but the tip lacked
  headers. The final rewrite normalizes each historical snapshot, preserving
  headers from introduction through the tip.

- Observation: `CMakeLists.txt` accepts `#` comments, not C-style block
  comments.
  Evidence: configuring the first rewrite failed with `Expected a command
  name, got unquoted argument with text "/*"`. The final rewrite uses hash
  SPDX comments for all CMake files and the desktop build then succeeds.

- Observation: The original history is linear and has no tags or merge commits.
  Evidence: both the original and rewritten histories have 29 commits and zero
  commits with two or more parents.

## Decision Log

- Decision: Use `SPDX-FileCopyrightText` and `SPDX-License-Identifier` instead of prose-only copyright notices.
  Rationale: SPDX headers are concise, machine-readable, widely supported, and easier to validate consistently.
  Date: 2026-07-13

- Decision: Use the exact copyright line:

      SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.

  Rationale: This is the copyright holder and year explicitly required for this repository-wide standardization.
  Date: 2026-07-13

- Decision: Use `LGPL-2.1-only`.
  Rationale: The requested license is LGPL 2.1. The `-only` suffix prevents unintentionally granting use under later LGPL versions. If the existing authoritative license text explicitly grants “version 2.1 or any later version,” stop and record that discovery before changing the identifier to `LGPL-2.1-or-later`.
  Date: 2026-07-13

- Decision: Identify Fabio Sobral as `Creator and Lead Maintainer` in repository-level documentation.
  Rationale: Project authorship and current maintenance should be visible without changing the legal copyright holder.
  Date: 2026-07-13

- Decision: Do not add personal `@author` tags or maintainer comments to every source file.
  Rationale: Per-file authorship tags become stale, duplicate repository metadata, and can be confused with legal ownership. Visibility is better provided through the README, `AUTHORS.md`, `CODEOWNERS`, and Git history.
  Date: 2026-07-13

- Decision: Do not replace or rewrite third-party copyright notices.
  Rationale: Vendored, imported, generated, or separately licensed files retain their original ownership and licensing information.
  Date: 2026-07-13

- Decision: Validation must be deterministic and runnable locally as well as in CI.
  Rationale: Contributors should discover copyright problems before submitting a pull request.
  Date: 2026-07-13

- Decision: Rewrite history only in a disposable clone or dedicated local rewrite branch with backup refs.
  Rationale: Commit IDs, signatures, tags, and downstream clones are affected by history rewriting, so the original history must remain recoverable until validation and explicit publication approval.
  Date: 2026-07-13

- Decision: Preserve original Git authorship and commit metadata.
  Rationale: Adding repository attribution must not falsely reassign commit authorship or erase contributor history.
  Date: 2026-07-13

- Decision: Add SPDX headers at each file's historical introduction commit.
  Rationale: The repository should be license-compliant throughout the rewritten history rather than only at the final tree.
  Date: 2026-07-13

- Decision: Keep the rewritten history on the local `spdx-history` branch and
  leave `main` at the original tip.
  Rationale: This satisfies the dedicated-branch safety requirement and avoids
  a force-push or changing collaborators' local history without separate
  publication approval.
  Date: 2026-07-14

- Decision: Treat C/C++/Objective-C, CMake, Gradle, shell, YAML, XML, HTML,
  Python, and `android/gradle.properties.example` as first-party applicable
  files; exclude documentation, binaries, generated/build directories, vendor
  directories, and the Gradle wrapper.
  Rationale: These are the repository's implementation and build inputs that
  safely support comments. Documentation and third-party artifacts do not need
  source-file SPDX headers.
  Date: 2026-07-14

## Outcomes & Retrospective

Complete this section after implementation.

Summarize:

- how many files were inspected;
- how many files received canonical SPDX headers;
- how many files were excluded and why;
- whether any licensing inconsistency was discovered;
- the validation command added;
- the CI workflow or job updated;
- the build and test commands executed;
- remaining limitations or follow-up work;
- number of commits rewritten;
- whether merge topology and author metadata were preserved;
- location of the old-to-new commit mapping;
- impact on tags, signatures, forks, and open pull requests.

Completed on the local `spdx-history` branch.

- Inspected 70 tracked files in the original tip. Of 59 applicable historical
  first-party source/build files, all received the canonical headers. The
  Gradle wrapper, its binary JAR, documentation, and other non-source files
  were excluded; third-party wrapper notices remain untouched.
- Added the full LGPL 2.1 license text, `AUTHORS.md`, default
  `.github/CODEOWNERS`, README maintainer attribution, contributor guidance,
  a dependency-free validator, 12 fixture tests, and a dedicated CI workflow.
- The local command is `python3 scripts/check_copyright.py`; it reports
  `Copyright validation passed: 62 applicable files checked.` at the final
  tip, including the validator and workflow themselves.
- `python3 scripts/test_check_copyright.py` passed all 12 tests. The focused
  SDL3 + Skia CPU CMake configure and build completed successfully in
  `build-spdx`.
- The original 29 commits were rewritten to the local `spdx-history` branch.
  `refs/backup/pre-spdx-main` retains the original tip. The original history
  has no merge commits or tags; the rewrite preserves its linear topology and
  author/committer names, emails, timestamps, and subjects. The commit map is
  `docs/history/spdx-copyright-rewrite-2026-07-14.md`.
- No rewritten history was pushed. Any existing clones, forks, open pull
  requests, or signatures would require separate coordination before a
  force-push or publication of `spdx-history`.

## Context and Orientation

Before modifying files, inspect the repository rather than assuming its layout.

Start with:

    pwd
    find .. -name AGENTS.md -o -name PLANS.md
    git status --short
    find . -maxdepth 3 -type f | sort | sed -n '1,240p'

Read all applicable `AGENTS.md` files before editing anything. Read the root `PLANS.md` if present.

Inspect licensing and attribution files:

    find . -maxdepth 3 -type f \( \
      -iname 'LICENSE*' -o \
      -iname 'COPYING*' -o \
      -iname 'NOTICE*' -o \
      -iname 'AUTHORS*' -o \
      -iname 'README*' -o \
      -iname 'CONTRIBUTING*' -o \
      -iname 'CODEOWNERS' \
    \) -print

Inspect existing copyright and license markers:

    git grep -n -I -E \
      'Copyright|SPDX-FileCopyrightText|SPDX-License-Identifier|Lesser General Public License|LGPL' \
      -- . \
      ':(exclude).git/**' \
      ':(exclude)build/**' \
      ':(exclude)dist/**' \
      ':(exclude)out/**' \
      ':(exclude)target/**' \
      ':(exclude)node_modules/**' \
      || true

Inspect CI and build entry points:

    find .github -maxdepth 3 -type f -print 2>/dev/null | sort
    find . -maxdepth 2 -type f \( \
      -name 'build.gradle' -o \
      -name 'build.gradle.kts' -o \
      -name 'settings.gradle' -o \
      -name 'settings.gradle.kts' -o \
      -name 'CMakeLists.txt' -o \
      -name 'Makefile' -o \
      -name 'pom.xml' -o \
      -name 'package.json' \
    \) -print

Do not assume that every tracked text file should receive a header. Classify files before migration.

Relevant categories normally include:

- first-party Java, Kotlin, C, C++, Objective-C, Objective-C++, Rust, Python, shell, Gradle, Groovy, JavaScript, TypeScript, and similar implementation files;
- first-party build scripts whose syntax supports comments;
- first-party test source files;
- first-party example source files.

Files normally excluded include:

- vendored or subtree-managed third-party source;
- files with existing third-party copyright;
- generated files;
- build outputs;
- minified assets;
- lockfiles;
- binary files;
- patch files;
- external fixtures copied verbatim;
- license texts;
- files whose format does not safely permit comments;
- files explicitly excluded by repository policy.

Repository documentation should make these exclusions understandable and maintainable.

## Canonical SPDX headers

Use the shortest valid comment syntax for each file format while preserving mandatory first-line constructs.

### C-style source files

For Java, Kotlin, C, C++, Objective-C, Objective-C++, JavaScript, TypeScript, Gradle, Groovy, and other formats supporting block comments:

    /*
     * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
     * SPDX-License-Identifier: LGPL-2.1-only
     */

Prefer this format for consistency unless repository conventions require line comments.

### Hash-comment source files

For shell, Python, Ruby, YAML, and similar formats:

    # SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
    # SPDX-License-Identifier: LGPL-2.1-only

When a file starts with a shebang, preserve the shebang as the first line and place the SPDX header immediately after it:

    #!/usr/bin/env bash
    # SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
    # SPDX-License-Identifier: LGPL-2.1-only

Preserve Python encoding declarations in their required location.

### XML-style files

For XML and other compatible formats, preserve the XML declaration as the first line when present, then use:

    <!--
      SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
      SPDX-License-Identifier: LGPL-2.1-only
    -->

Do not add comments to formats where doing so could change parsing semantics.

### Existing historical years

The requested canonical first-party copyright line uses `2026`. Do not invent earlier years from Git history.

If an existing first-party file contains a legally meaningful historical range, such as `2001-2026`, do not silently discard it. Record the finding in `Surprises & Discoveries` and determine whether the repository-wide requirement is intended to replace historical notices or only standardize newly owned work.

Unless repository evidence clearly requires preservation, use the exact requested canonical line:

    SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.

Never modify historical years belonging to third parties.

## Repository-level attribution

### AUTHORS.md

Create or update `AUTHORS.md` with concise attribution.

The file should contain content equivalent to:

    # Authors

    ## Creator and Lead Maintainer

    Fabio Sobral
    GitHub: [@flsobral](https://github.com/flsobral)

    ## Copyright holder

    Amalgam Solucoes em TI Ltda.

    ## Contributors

    Additional contributors are recorded in the Git history.

    Copyright ownership and project authorship are separate concepts. Unless
    explicitly stated otherwise, contributions are licensed under the
    repository's LGPL 2.1 license.

Adapt the last paragraph to the repository's actual contribution policy. Do not make claims about copyright assignment or contributor license agreements unless the repository contains authoritative documentation supporting those claims.

### README.md

Add a visible but restrained maintainer section near the project introduction, community section, or contribution section.

Use wording equivalent to:

    ## Maintainer

    Created and maintained by [Fabio Sobral](https://github.com/flsobral).

    Copyright © 2026 Amalgam Solucoes em TI Ltda.

Do not imply that Fabio Sobral personally owns the company copyright.

If the README already contains an authorship or governance section, update it rather than creating a duplicate.

### CODEOWNERS

Create or update `.github/CODEOWNERS` so that Fabio Sobral is the default code owner:

    * @flsobral

Preserve any existing path-specific ownership rules. Since later matching rules take precedence in GitHub CODEOWNERS syntax, place the repository-wide fallback where it does not unintentionally override more specific rules.

Do not enable branch protection or mandatory code-owner review unless explicitly requested. This plan adds ownership metadata only.

## Contributor documentation

Update the most appropriate contributor-facing file, normally `CONTRIBUTING.md`, with a section equivalent to:

    ## Copyright headers

    New first-party source files must include the following SPDX metadata:

        SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
        SPDX-License-Identifier: LGPL-2.1-only

    Use the comment syntax appropriate for the file type. Preserve shebangs,
    XML declarations, encoding declarations, and other required first-line
    constructs.

    Do not replace or modify copyright and license notices in third-party,
    vendored, generated, or separately licensed files.

    Run the repository copyright validation command before submitting a pull
    request.

Document the exact local command after implementing the validation tool.

If the repository contains templates used to create source files, update them to emit the canonical SPDX header.

## Validation tool

Implement the validator using an existing repository language where practical. Prefer a small dependency-free script over introducing a new runtime or package manager.

Good locations include:

    tools/check-copyright.py
    scripts/check-copyright.py
    tools/check-copyright.sh

Prefer Python when it is already available in CI because it offers reliable path handling and readable diagnostics. If the project deliberately avoids Python, use the repository's existing scripting language.

The validator must:

1. obtain candidate files from Git rather than recursively scanning build output;
2. inspect tracked files and, when useful, staged files;
3. apply explicit inclusion and exclusion rules;
4. detect third-party and generated directories;
5. verify both canonical SPDX lines;
6. verify the exact company spelling;
7. verify the expected license identifier;
8. identify obsolete first-party company notices that should have been migrated;
9. print one concise diagnostic per failing file;
10. exit with status zero on success and nonzero on failure;
11. produce deterministic output sorted by path;
12. support paths containing spaces;
13. avoid rewriting files during validation.

A suitable candidate-file source is:

    git ls-files -z

Do not parse newline-delimited file lists when NUL-delimited output is available.

Diagnostics should resemble:

    path/to/File.java: missing SPDX-FileCopyrightText
    path/to/File.java: expected copyright holder "2026 Amalgam Solucoes em TI Ltda."
    path/to/File.java: expected SPDX license "LGPL-2.1-only"
    vendor/library.c: excluded as third-party

Routine exclusions need not be printed during successful validation. A summary may report counts.

A successful run should resemble:

    Copyright validation passed: 842 applicable files checked.

Keep output concise because the command will run in CI and may be consumed by automated agents.

## Tests for the validator

Add lightweight automated tests or fixture-based checks.

At minimum, cover:

1. valid C-style header;
2. valid hash-comment header;
3. shebang followed by a valid header;
4. missing copyright line;
5. missing license line;
6. misspelled company name;
7. wrong year;
8. `LGPL-2.1-or-later` when `LGPL-2.1-only` is expected;
9. third-party file exclusion;
10. generated-file exclusion;
11. path containing spaces;
12. deterministic sorted diagnostics.

Use the repository's existing test framework when one exists. Otherwise, a dependency-free test script is sufficient.

## CI integration

Inspect existing workflows and reuse their conventions.

Add a focused job or step named similarly to:

    copyright
    license-headers
    validate-copyright

The CI step must invoke the same command documented for local use.

## Validation and Acceptance

The implementation is complete only when all of the following are observable.

- Every applicable first-party source file contains the exact required SPDX copyright and license lines.
- Third-party, vendored, generated, and separately licensed files retain their original notices.
- `README.md` identifies Fabio Sobral as creator and maintainer.
- `AUTHORS.md` identifies Fabio Sobral as Creator and Lead Maintainer and Amalgam Solucoes em TI Ltda. as copyright holder.
- `.github/CODEOWNERS` contains an appropriate default rule for `@flsobral`.
- The documented local validation command succeeds from a clean checkout.
- CI executes the same validator.
- The normal focused build and test suite continue to pass.
- The final diff contains no unrelated changes.

## Final Review Checklist

- [ ] The authoritative license grant matches `LGPL-2.1-only`.
- [ ] The company name is exactly `Amalgam Solucoes em TI Ltda.`.
- [ ] The year is exactly `2026`.
- [ ] Fabio Sobral is described as creator and lead maintainer, not as copyright holder.
- [ ] Third-party notices were preserved.
- [ ] Generated files were not manually modified.
- [ ] New-file rules are documented.
- [ ] The validator is deterministic.
- [ ] Validator tests pass.
- [ ] CI invokes the validator.
- [ ] The documented local command works.
- [ ] Focused repository tests pass.
- [ ] `git diff --check` passes.
- [ ] The final diff contains no unrelated changes.
- [ ] Backup refs for the original history exist and remain intact.
- [ ] The rewritten initial commit contains LICENSE, AUTHORS.md, and CODEOWNERS.
- [ ] Each applicable file contains the canonical SPDX header from its introduction commit onward.
- [ ] Original author and committer metadata are preserved.
- [ ] Merge topology is preserved or any intentional deviation is documented.
- [ ] Old-to-new commit mapping is generated and validated.
- [ ] No rewritten history has been force-pushed without separate explicit authorization.
