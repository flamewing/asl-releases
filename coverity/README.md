# Coverity formatter model candidate

Upload **model.c**, by itself, through the project's Modeling File form.
This is a candidate, not an analyzer-validated fix. It is excluded from the
application build. Committing it does not upload or activate it; the existing
workflow uploads capture data and leaves analysis to hosted Coverity Scan.

The modeled function is `as_snprintf` (there is no `as_sprintf` in this revision).
The model describes its positive-capacity termination guarantee, conditional
zero-capacity no-access behavior, fatal pointer-size guard, initialized output
prefix, actual-stored-count return range, and propagation of input taint.
It neither asserts that all 1024 destination bytes are initialized nor replaces
the custom format grammar with libc formatting. It deliberately models only
this entry point; append and va_list variants retain their derived models.

Model constraints use comparisons rather than arithmetic, as required by the
[modeling documentation](https://docs.blackduck.com/r/coverity/latest/coverity-documentation/adding-a-custom-model.html?contentId=O3VsJ_Z4UU7qfNl~AQ2KDw).
The bounded write and kill-path primitives are documented under
[resource-management primitives](https://docs.blackduck.com/r/coverity/2026.3/coverity-documentation/c/c-resource-management-primitives.html?contentId=rIljj5gmdfdousHIUWPp1g);
format-string termination sinks and taint propagation are documented under
[security primitives](https://docs.blackduck.com/r/coverity/latest/coverity-documentation/c/c-security-primitives.html?contentId=dyLPapCOFrkmAX65DWQvtw).

## Acceptance before general use

The repository's native formatter tests establish runtime behavior, not whether
Coverity compiles or interprets this model correctly. No `cov-make-library` or
`cov-analyze` is installed in the development workspace. A GCC syntax-only
check does not validate the model's analyzer semantics.

Local checks completed: GCC 13.3.0 C11 syntax-only checks for `model.c` and
`probes.c`, and the existing `formatter_contracts` runtime test, all pass.

Analyze the same revision/configuration with and without the model. Include
`probes.c` in a separate diagnostic capture with the repository include path
and real formatter implementation; never execute its invalid probes. Confirm:

- The banner and one-byte positive cases are accepted, including CID 915445.
- Zero capacity does not dereference any input or output.
- Null output, oversized output capacity, unterminated format, and unterminated
  `%s` arguments still produce diagnostics.
- Wrong argument types and missing format arguments remain detectable. Existing
  custom radix and LargeWord conversions must not gain libc-grammar errors.
- Bytes beyond the stored output and its NUL remain potentially uninitialized.

The model does **not** parse variadic conversions. Its taint propagation is not
a substitute for validating `%s` input strings or checking argument types. In
particular, replacement of the derived model may lose such diagnostics. If the
negative probes disappear, reject/disable this candidate and ask Scan support
for a checker-specific correction or a model preserving the format semantics.
Do not accept disappearance of CID 915445 alone as sufficient validation.

If upload rejects the target-dependent `size_t` typedef or any primitive,
retain the diagnostic and ask support which model dialect/version the hosted
project uses. Do not silently replace it with an arbitrary integer type.

## Capacity inference and vendor report

The expanded events identify the displayed `1` as the byte count in
`append(p_dest_ctx, pFormat, 1, &FormatContext)`, which handles a literal
character. Inside `append`, `memcpy(..., p_src, cnt)` intentionally copies
that byte without a NUL. The string-copy summary carries this count back to
the public call. It is not evidence of `sizeof(Tmp)` becoming 1.

The decisive false branch is in `vsprcatf_core`: Coverity takes
`dest_remlen > 0` as false and therefore skips the final NUL. For this
replacement-format call, the initially empty destination has positive
remaining capacity. Both `append` and `append_pad` use `limit_minus_one`,
which limits each write to at most remaining capacity minus one. Thus
positive remaining capacity stays positive, even after truncation. The
shown zero-capacity exit is infeasible for this call.

The trace also takes `arg[0] > cnt` as true for the literal-character append.
For the initial `(`, reset sets the width to zero and `cnt` is 1, so that
branch is also infeasible in the concrete banner call. These events point
to lost invariants/correlations in helper summaries, rather than an array
size inference error. The exact analyzer mechanism still requires vendor
investigation; this has not been reproduced with a local analyzer.

[The Scan contact page](https://scan.coverity.com/contact) offers administrator
contact and community support. `report-915445.txt` is a prepared report, not a
submitted ticket. Include the screenshot, snapshot/revision from Coverity,
capture-tool version, and hosted analyzer version when known. A capture-tool
version alone does not establish the analyzer version.
