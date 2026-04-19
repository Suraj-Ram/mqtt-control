---
name: Small steps preference
description: User wants changes broken into very small, focused steps — one thing at a time, stop and wait
type: feedback
---

Do not make large sweeping changes in a single turn. When refactoring or updating code, make one small focused change, then stop and wait for the user to confirm before proceeding.

**Why:** User explicitly asked for "very small steps" and was frustrated when a single turn added a new struct, modified LampState, added a new method, and rewrote a function all at once.

**How to apply:** Before each edit, ask yourself: is this the single smallest compilable change that moves toward the goal? If a step would break compilation and force a follow-up fix in the same turn, reconsider the step boundary. But don't split changes that are truly coupled (e.g. removing a variable and all its references must happen together).
