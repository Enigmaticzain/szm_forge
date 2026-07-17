for f in targets:
    log(f"\n--- FILE: {f} ---")

    code = read_file(f)

    log("Running EXECUTOR...")
    new_code = executor(task, step, f, code, memory)

    if not new_code or len(new_code) < 20:
        log("Skipped: invalid output")
        continue

    log("Running REVIEWER...")
    review = reviewer(task, step, new_code)
    log(f"Reviewer: {review}")

    if "FIX" in review:
        log("Rejected by reviewer")
        continue

    log("Running SANDBOX...")
    rc, out, err = sandbox_run(new_code)

    if rc != 0:
        log("Sandbox error detected")
        log(err[:200])

        fix = ask_llm(f"""
Fix this code.

Error:
{err}

Return full corrected code.
""")

        if fix and len(fix) > 20:
            new_code = fix
            log("Auto-fixed code")

    diff = "\n".join(
        list(difflib.unified_diff(code.splitlines(), new_code.splitlines()))[:50]
    )
    log(f"DIFF:\n{diff}")

    write_file(f, new_code)
    log(f"Applied changes to {f}")

    store_memory(f"{task} → {f}")
