# LogHarbor

LogHarbor is an offline C++17 toolkit for inspecting local logs, normalizing records, building incident timelines, querying stored events, and redacting sensitive text before export.

## Use cases

- Triage a directory of rotated application, web, and system logs without sending data to a service.
- Convert mixed local logs into JSON Lines for review tools.
- Build a compact local record store, then run repeatable filters across incident evidence.
- Redact email addresses, IP addresses, passwords, tokens, and API keys before sharing excerpts.

## Supported inputs

- RFC 5424/syslog-style lines with host, service tag, severity inference, and key/value extraction.
- Apache and Nginx common or combined access logs.
- JSON Lines application logs with flat objects.
- CSV event exports with a header row.
- INI-style configuration files represented as configuration events.
- Chunked or rotated streams with continuation records.

## Architecture

- `core` provides result types, diagnostics, byte readers, checksums, string utilities, time parsing, and safe slicing.
- `ingest` parses real log formats and normalizes records into a common event model.
- `index` stores normalized events with varints, record pages, tombstone handling, compaction, and replayable metadata.
- `query` lexes, parses, and evaluates field filters, string matches, boolean expressions, numeric ranges, and time-shaped predicates.
- `stream` decodes partial reads, rotated segments, framed records, continuations, and reconstructed messages.

## CLI usage

```text
logscan app.log
logindex incident.idx app.log access.log events.jsonl
logquery incident.idx "severity=error and status>=500"
logredact app.log
```

## Build

```text
cmake -S . -B build
cmake --build build
```

## Test

```text
ctest --test-dir build --output-on-failure
```

## Developer fuzzing

The `fuzz/` directory contains libFuzzer-compatible harnesses for ingestion, storage decoding, query parsing, and stream reconstruction. They are developer QA entry points and call the same library code used by the command-line tools.

```text
cmake -S . -B build-fuzz -DLOGHARBOR_BUILD_FUZZERS=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build-fuzz
```

The seed corpus contains representative syslog, web, JSONL, CSV, store, query, stream, and malformed near-valid inputs. `fuzz/dictionary.txt` lists useful tokens for timestamps, HTTP fields, JSON/CSV punctuation, query operators, and stream frame markers.

## Manual review checklist

- Confirm parsers reject malformed inputs without losing valid adjacent records.
- Review record-store compatibility before changing serialized fields.
- Check redaction output against the local sharing policy for the incident.
- Keep all processing offline and deterministic.
- Prefer focused tests whenever changing parser, query, or compaction behavior.
