# GitHub Actions Setup Guide

## Quick Start

### 1. Configure Discord Notifications (Optional)

1. Create Discord webhook:
   - Server Settings → Integrations → Webhooks → New Webhook
   - Name: "OpenFIDO-ESP Bot"
   - Copy webhook URL

2. Add to GitHub Secrets:
   - Repository → Settings → Secrets and variables → Actions
   - New repository secret
   - Name: `DISCORD_WEBHOOK_URL`
   - Value: (paste webhook URL)

### 2. Test CI/CD Pipeline

```bash
# Make a small change
echo "# Test" >> README.md
git add README.md
git commit -m "test: verify CI/CD pipeline"
git push origin main
```

Check **Actions** tab - you should see 4 jobs running:
- ✅ Build Firmware
- ✅ Run Tests  
- ✅ Code Quality & Static Analysis
- ✅ Code Coverage Analysis

### 3. Create Your First Release

```bash
# Make changes with conventional commits
git commit -m "feat: initial U2F implementation"
git commit -m "fix: resolve authentication issue"

# Create and push tag
git tag v0.1.0
git push origin v0.1.0
```

This will:
1. Build optimized firmware
2. Generate changelog from commits
3. Create GitHub Release
4. Send Discord notification (if configured)

---

## Conventional Commits

Use these prefixes for automatic changelog generation:

| Prefix | Category | Example |
|--------|----------|---------|
| `feat:` | Features | `feat: add FIDO2 support` |
| `fix:` | Bug Fixes | `fix: resolve USB enumeration` |
| `docs:` | Documentation | `docs: update README` |
| `perf:` | Performance | `perf: optimize crypto operations` |
| `refactor:` | Refactoring | `refactor: restructure U2F module` |
| `test:` | Testing | `test: add unit tests for CBOR` |
| `chore:` | Miscellaneous | `chore: update dependencies` |

---

## Troubleshooting

### Workflow fails on first run

**Cause**: Missing firmware files or configuration  
**Solution**: Ensure `firmware/` directory exists with valid ESP-IDF project

### Discord notification not received

**Cause**: Secret not configured  
**Solution**: Add `DISCORD_WEBHOOK_URL` secret (see step 1)

### Lint job fails

**Cause**: Code formatting issues  
**Solution**: Run `find firmware/main -name '*.c' -o -name '*.h' | xargs clang-format -i`

---

## Next Steps

- [ ] Configure Discord webhook for notifications
- [ ] Make first commit with conventional format
- [ ] Create first release tag
- [ ] Review coverage reports in artifacts
