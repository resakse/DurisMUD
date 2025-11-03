# DurisMUD Security Remediation Plan

**Status:** In Progress
**Timeline:** 4-6 Months (Incremental)
**Approach:** All CRITICAL severity fixes, breaking changes acceptable, keep all features running

---

## Security Audit Summary

**Total Vulnerabilities Identified:** 23

**By Severity:**
- ❌ **CRITICAL:** 4 (SQL Injection x2, Format String, Command Injection)
- ❌ **HIGH:** 6 (Weak passwords, Buffer overflows, Integer overflow, Privilege escalation, Unsafe strings)
- ❌ **MEDIUM:** 9 (Race conditions, Info disclosure, DoS, Path traversal, Input validation)
- ❌ **LOW:** 4 (Magic numbers, Memory management, Deprecated functions, Timing attacks)

---

## Phase 1: SQL Injection Fixes (Weeks 1-3)

**Month 1 - Critical Database Security**

### Tasks

- [x] **Fix Wiki Search SQL Injection** (src/sql.c:1004) ✅ **COMPLETED 2025-11-03**
  - [x] Implement `mysql_real_escape_string()` wrapper function
  - [x] Sanitize all user input before query construction in `perform_wiki_search()`
  - [ ] Add input validation (max length 256, alphanumeric + spaces only) - **TODO: Next iteration**
  - [ ] Add rate limiting to prevent abuse (5 searches per minute per player) - **TODO: Next iteration**
  - [ ] Test with malicious inputs: `'); DROP TABLE--`, `' OR '1'='1`, etc. - **TODO: User testing required**

- [x] **Secure do_sql Command** (src/sql.c:1213-1393) ✅ **COMPLETED 2025-11-03**
  - [ ] Audit all uses of prepared statements feature - **TODO: Next iteration**
  - [ ] Add additional god-level check (only allow level 62+) - **TODO: Next iteration**
  - [x] Sanitize description field before INSERT/UPDATE (line 1323)
  - [x] Sanitize sql_code field before qry() call (line 1329)
  - [x] Add comprehensive logging of all SQL command usage - **Already exists (line 1250-1252)**
  - [ ] Test prepared statement feature for secondary injection - **TODO: User testing required**

- [ ] **Database Security Hardening**
  - [ ] Create `safe_db_query()` wrapper that validates inputs - **TODO: Next iteration**
  - [ ] Implement prepared statements for all high-risk queries - **TODO: Next iteration**
  - [ ] Add query logging/monitoring for suspicious patterns - **TODO: Next iteration**
  - [ ] Review all 36 uses of `db_query()` function throughout codebase - **TODO: Next iteration**
  - [ ] Document safe query practices for future development - **TODO: Next iteration**

**Success Criteria:** Zero SQL injection vulnerabilities, comprehensive logging active

---

## Phase 2: Format String & Buffer Overflows (Weeks 4-7)

**Month 2 - Memory Safety Critical Path**

### Tasks

- [x] **Fix vsprintf() Format String Vulnerability** (src/sql.c:268-287) ✅ **COMPLETED 2025-11-03**
  - [x] Replace `vsprintf(buf, format, args)` with `vsnprintf(buf, sizeof(buf), format, args)` in `db_query()`
  - [x] Replace `vsprintf()` with `vsnprintf()` in `db_query_nolog()` (line 301)
  - [x] Replace `vsprintf()` with `vsnprintf()` in `qry()` (line 1079)
  - [x] Add overflow checking after vsnprintf in all three functions
  - [ ] Audit all other variadic functions for similar issues - **TODO: Next iteration**
  - [ ] Test with format string exploits: `%n%n%n%n`, `%s%s%s%s`, etc. - **TODO: User testing required**

- [ ] **Eliminate Remaining sprintf() Calls** (9 instances)
  - [ ] Fix src/ships/ship_control.c
  - [ ] Fix src/ships/ship_utils.c
  - [ ] Fix src/ships/ship_shop.c
  - [ ] Fix src/ships/ship_base.c
  - [ ] Fix src/utility.c
  - [ ] Fix src/sql.c
  - [ ] Fix src/ships/ship_npc.c
  - [ ] Fix src/ships/ship_cargo.c
  - [ ] Add `-Werror=format-overflow` to Makefile to prevent future sprintf

- [ ] **strcpy/strcat Mass Replacement** (97 files)
  - [ ] Create safe wrapper functions in utility.c:
    - [ ] `safe_strcpy(dest, src, size)` - strcpy with bounds
    - [ ] `safe_strcat(dest, src, size)` - strcat with bounds
    - [ ] Add runtime assertions for buffer overruns
  - [ ] Replace in network code (comm.c, mccp.c, sound.c)
  - [ ] Replace in command handlers (interp.c, act.*.c, do_*.c)
  - [ ] Replace in database code (db.c, db.mysql.c, sql.c)
  - [ ] Replace in remaining files (batched by subsystem)
  - [ ] Add compile-time deprecation warnings for strcpy/strcat

**Success Criteria:** Zero format string vulnerabilities, all string operations bounded

---

## Phase 3: Command Injection & Privilege Escalation (Weeks 8-10)

**Month 3 - Access Control & System Security**

### Tasks

- [ ] **Audit system()/popen()/exec() Calls** (11 files)
  - [ ] Map all command execution points:
    - [ ] nanny.c
    - [ ] modify.c
    - [ ] quest.c
    - [ ] actwiz.c
    - [ ] files.c
    - [ ] properties.c
    - [ ] fraglist.c
    - [ ] comm.c
    - [ ] actoth.c
    - [ ] account.c
  - [ ] Verify no user input reaches these functions
  - [ ] Add input sanitization where user data is involved
  - [ ] Replace with safer alternatives (avoid shell execution)
  - [ ] Document all command execution points

- [ ] **Fix do_advance Privilege Escalation** (src/actwiz.c:5290+)
  - [ ] Add upper bound check: `if (newlevel > MAX_IMMORTAL_LEVEL)`
  - [ ] Prevent gods from advancing themselves: `if (victim == ch)`
  - [ ] Require advancer's level > target level: `if (GET_LEVEL(ch) <= newlevel)`
  - [ ] Add comprehensive logging of all advancement attempts
  - [ ] Test privilege escalation scenarios
  - [ ] Review other god commands for similar issues

- [ ] **Integer Overflow Protection**
  - [ ] Create `safe_atoi(str, min, max)` wrapper with bounds checking
  - [ ] Replace atoi() in critical paths:
    - [ ] Level conversions
    - [ ] Vnum parsing
    - [ ] Money/gold conversions
    - [ ] Stat modifications
  - [ ] Add overflow detection in arithmetic (levels, money, stats)
  - [ ] Use `strtol()` with error checking instead of atoi()

**Success Criteria:** No privilege escalation, command injection eliminated, bounded integer conversions

---

## Phase 4: Password Security Upgrade (Weeks 11-14)

**Month 4 - Authentication Overhaul**

### Tasks

- [ ] **Implement Modern Password Hashing**
  - [ ] Add bcrypt library dependency to Makefile.linux
  - [ ] Create new password hashing functions:
    - [ ] `char *hash_password_bcrypt(const char *password)` - cost factor 12
    - [ ] `int verify_password_bcrypt(const char *password, const char *hash)`
  - [ ] Implement migration path (check pwd[0] for prefix like existing '$' check)
  - [ ] Add '#' prefix for bcrypt hashes to distinguish from old format
  - [ ] Support both old and new hashes during transition

- [ ] **Password System Changes**
  - [ ] Replace DES `crypt()` calls in nanny.c:4411-4412
  - [ ] Replace CRYPT2 calls in account.c:144
  - [ ] Implement constant-time password comparison
  - [ ] Update password change commands to use new hashing
  - [ ] Force password reset for all accounts on first login
  - [ ] Update both character and account password systems

- [ ] **Session Security**
  - [ ] Add failed login attempt tracking (per IP and per account)
  - [ ] Implement account lockout after 5 failures (30 min timeout)
  - [ ] Add session timeout mechanisms (idle disconnect)
  - [ ] Log all authentication events to security.log
  - [ ] Add brute force detection

**Success Criteria:** Modern password hashing (bcrypt), account lockout protection, no plaintext passwords

---

## Phase 5: Input Validation & Race Conditions (Weeks 15-18)

**Month 5 - Robustness & Data Integrity**

### Tasks

- [ ] **Comprehensive Input Validation**
  - [ ] Audit all command handlers (800+ commands in interp.c)
  - [ ] Add length checks before string operations
  - [ ] Validate ranges for numeric inputs
  - [ ] Sanitize special characters in:
    - [ ] Player names
    - [ ] Object descriptions
    - [ ] Room titles
    - [ ] Guild names
    - [ ] Wiki content
  - [ ] Add input validation helper functions
  - [ ] Document validation requirements

- [ ] **Fix Race Conditions**
  - [ ] Add database transactions to `sql_find_racewar_for_ip()` (sql.c:945-984)
  - [ ] Use SELECT FOR UPDATE where appropriate
  - [ ] Review all multi-step database operations
  - [ ] Add proper locking for shared state (guild data, racewar)
  - [ ] Fix TOCTOU vulnerabilities
  - [ ] Test concurrent connection scenarios

- [ ] **Path Traversal Prevention** (15+ files with fopen)
  - [ ] Audit all fopen() calls in:
    - [ ] randomeq.c
    - [ ] outposts.c
    - [ ] random.zone.c
    - [ ] statistics.c
    - [ ] nanny.c
    - [ ] debug.c
    - [ ] modify.c
    - [ ] guild.c
    - [ ] quest.c
    - [ ] actwiz.c
    - [ ] files.c
  - [ ] Add path validation (no ../, no absolute paths outside allowed dirs)
  - [ ] Create safe file open wrapper: `safe_fopen(filename, mode, allowed_dir)`
  - [ ] Sanitize all filenames from user input
  - [ ] Test path traversal: `../../etc/passwd`, `../../../root/`, etc.

**Success Criteria:** Robust input validation on all commands, no race conditions, path traversal prevented

---

## Phase 6: Hardening & Monitoring (Weeks 19-24)

**Month 6 - Defense in Depth**

### Tasks

- [ ] **Rate Limiting & DoS Prevention**
  - [ ] Implement login attempt rate limiting (3 attempts per 10 seconds per IP)
  - [ ] Add command rate limiting per player (configurable by command)
  - [ ] SQL query rate limiting (prevent query spam)
  - [ ] Connection limits per IP (max 3 concurrent)
  - [ ] Add resource exhaustion protection

- [ ] **Security Logging & Monitoring**
  - [ ] Create centralized security event logging system
  - [ ] Log failed login attempts with IP/account
  - [ ] Implement suspicious command pattern detection
  - [ ] Create admin action audit trail (all god commands)
  - [ ] Add log rotation for security logs
  - [ ] Create log analysis tools

- [ ] **Code Quality Improvements**
  - [ ] Fix weak randomness (rand() → `/dev/urandom` or `getrandom()`)
  - [ ] Replace magic number in player wipe (sql.c:1669)
  - [ ] Add compile-time security flags to Makefile:
    - [ ] `-fstack-protector-strong` (stack canaries)
    - [ ] `-D_FORTIFY_SOURCE=2` (buffer overflow detection)
    - [ ] `-pie -fPIE` (position independent executable)
    - [ ] `-Wformat -Wformat-security` (format string warnings)
  - [ ] Audit memory pool implementation (mm.c) for use-after-free
  - [ ] Enable Electric Fence in debug builds

- [ ] **Network Security** (Optional)
  - [ ] Evaluate SSL/TLS library options (OpenSSL, GnuTLS)
  - [ ] Implement optional TLS support for telnet
  - [ ] Add STARTTLS-like negotiation
  - [ ] Document plaintext protocol risks
  - [ ] Create secure connection option for players

**Success Criteria:** Comprehensive security logging, hardened build, rate limiting active

---

## Implementation Strategy

### Per Phase Workflow

1. ✅ **Create feature branch** for phase
2. ✅ **Implement fixes** with inline comments documenting changes
3. ✅ **Manual testing** on running development server
4. ✅ **Code review** (user reviews all changes)
5. ✅ **Deploy to production** (no downtime, live patching)
6. ✅ **Monitor** for regressions (check logs, test gameplay)
7. ✅ **Merge to master** after user thorough testing approval

### Testing Protocol

- Each fix tested individually on running server
- No downtime required (live patching where possible)
- User performs thorough testing before commit (35 years experience)
- Rollback plan for each phase (git branches)
- Test with actual player accounts and game scenarios

### Breaking Changes

- **Password reset** (Phase 4) - all players must reset passwords on next login
- **Player file format** may change if needed for security
- **Database schema updates** as required (with migration scripts)
- **Configuration file changes** for new security settings

---

## Critical Vulnerabilities Detail

### 1. SQL Injection in Wiki Search (CRITICAL)
**File:** src/sql.c:1004
**Function:** `perform_wiki_search()`
**Code:**
```c
MYSQL_RES *db = db_query("SELECT REPLACE(...) FROM `wikki_text` WHERE ... LOWER(page_title) = REPLACE(LOWER('%s'), ' ', '_') ...", query);
```
**Exploit:** `wiki ') UNION SELECT password FROM players_core WHERE ('1'='1`
**Impact:** Full database compromise, password theft, data modification

### 2. SQL Injection in do_sql (CRITICAL)
**File:** src/sql.c:1213-1393
**Function:** `do_sql()`
**Code:**
```c
// Line 1319: Unsanitized INSERT
snprintf(buf, MAX_STRING_LENGTH, "UPDATE prepstatement_duris_sql SET description = '%s' WHERE id='%d'", rest, prep_statement);

// Line 1347: Direct query execution
if (mysql_real_query(DB, argument, strlen(argument)))
```
**Impact:** God account compromise = full database control, secondary injection via stored statements

### 3. Format String Vulnerability (CRITICAL)
**File:** src/sql.c:268-287
**Function:** `db_query()`
**Code:**
```c
char buf[MAX_LOG_LEN + MAX_STRING_LENGTH + 512];
va_list args;
va_start(args, format);
vsprintf(buf, format, args);  // NO BOUNDS CHECKING
```
**Impact:** Memory corruption, arbitrary code execution, information disclosure, server crash

### 4. Command Injection (CRITICAL)
**Files:** Multiple (nanny.c, modify.c, quest.c, actwiz.c, files.c, properties.c, fraglist.c, comm.c, actoth.c, account.c)
**Impact:** Arbitrary system command execution, server compromise, file system access

---

## Success Metrics

- [ ] Zero CRITICAL vulnerabilities remaining
- [ ] Zero HIGH severity issues
- [ ] All buffer operations bounded (no sprintf, strcpy, strcat)
- [ ] Modern password hashing (bcrypt) implemented
- [ ] Comprehensive security logging active
- [ ] All integer conversions validated
- [ ] No SQL injection possible
- [ ] Rate limiting on all sensitive operations
- [ ] Prepared for external security audit

---

## Risk Mitigation

✅ **Keep all features running throughout** (no disabling features)
✅ **Incremental changes** reduce regression risk
✅ **Extensive testing at each phase** (user validates all changes)
✅ **Git branches** allow easy rollback
✅ **User's 35 years experience** ensures proper validation
✅ **Phased approach** allows learning and adjustment
✅ **No downtime required** for most changes

---

## Progress Tracking

**Overall Progress:** 0/6 Phases Complete (8% Critical Fixes)

- [~] Phase 1: SQL Injection Fixes (40% - Critical vulnerabilities patched)
- [~] Phase 2: Format String & Buffer Overflows (20% - vsprintf vulnerabilities fixed)
- [ ] Phase 3: Command Injection & Privilege Escalation (0%)
- [ ] Phase 4: Password Security Upgrade (0%)
- [ ] Phase 5: Input Validation & Race Conditions (0%)
- [ ] Phase 6: Hardening & Monitoring (0%)

**Key:** [x] Complete | [~] In Progress | [ ] Not Started

---

## Notes

- This is a living document - update checkboxes as tasks complete
- Add notes/observations during implementation
- Document any new vulnerabilities discovered during fixes
- Track time spent per phase for future estimation
- Keep security considerations for all future development

### Implementation Notes (2025-11-03)

**Session 1 - Critical SQL Injection & Format String Fixes:**
- Fixed SQL injection in `perform_wiki_search()` by escaping query input with `mysql_real_escape_string()`
- Fixed SQL injection in `do_sql()` command for description and sql_code fields
- Fixed format string vulnerability in `db_query()`, `db_query_nolog()`, and `qry()` by replacing `vsprintf()` with `vsnprintf()`
- All changes compiled successfully with zero warnings
- Code now has overflow checking on all three variadic SQL functions
- **Next:** User testing required, then continue with remaining Phase 1 & 2 tasks

---

**Last Updated:** 2025-11-03 12:55
**Document Version:** 1.1
**Status:** Phase 1 & 2 In Progress - Critical vulnerabilities patched, user testing required
