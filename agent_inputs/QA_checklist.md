QA checklist & success criteria (for the agent to validate)

 All binaries build with make and no binaries are included in zip.

 For each run: CSV row created and raw perf output stored.

 Filenames follow MT25xxx_* and zip is <roll_num>_PA02.zip.

 Plot scripts embed hardcoded arrays copied from CSVs (per spec).

 Report contains AI usage and prompts exactly as used.

 Repo is public on Github and README contains roll no. comment.

Risks & mitigations (what the agent should watch for)

Kernel support for MSG_ZEROCOPY may be missing → implement clean fallback to one-copy and record detection.

Running perf counters requires privileges → ensure agent runs with sudo or requests elevated permissions.

Network namespaces needed (spec says run client/server on separate namespaces) → use ip netns add ns1 / ns2 and ip netns exec.

Page pinning and memory alignment may cause EBUSY or errors → add retry and robust logging.

Small checklist of deliverables to hand back to you

Public GitHub repo URL (GRS_PA02 folder).

<roll_num>_PA02.zip containing: source, Makefile, CSVs, plot .py files, README, report (PDF), AI prompts file.

Short run log with commit hashes and run IDs.