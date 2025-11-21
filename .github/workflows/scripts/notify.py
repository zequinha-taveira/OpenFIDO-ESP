#!/usr/bin/env python3
"""
Discord notification script for GitHub Actions
Sends release notifications to Discord webhook
"""

import os
import sys
import json
import requests
from datetime import datetime


def send_discord_notification(webhook_url, version, changelog, download_url, commit_sha):
    """Send notification to Discord webhook"""
    
    # Create embed
    embed = {
        "title": f"🚀 OpenFIDO-ESP {version} Released!",
        "description": "New production firmware release available",
        "color": 0x00ff00,  # Green
        "fields": [
            {
                "name": "📦 Version",
                "value": version,
                "inline": True
            },
            {
                "name": "📅 Date",
                "value": datetime.utcnow().strftime("%Y-%m-%d %H:%M UTC"),
                "inline": True
            },
            {
                "name": "🔗 Download",
                "value": f"[GitHub Release]({download_url})",
                "inline": False
            },
            {
                "name": "📝 Changelog",
                "value": changelog[:1000] if changelog else "No changelog available",
                "inline": False
            },
            {
                "name": "🔨 Commit",
                "value": f"`{commit_sha[:7]}`",
                "inline": True
            }
        ],
        "footer": {
            "text": "OpenFIDO-ESP • U2F/FIDO2 Security Token"
        },
        "timestamp": datetime.utcnow().isoformat()
    }
    
    payload = {
        "content": "**New Release Available!** 🎉",
        "embeds": [embed],
        "username": "OpenFIDO-ESP Bot",
        "avatar_url": "https://raw.githubusercontent.com/FortAwesome/Font-Awesome/6.x/svgs/solid/key.svg"
    }
    
    try:
        response = requests.post(
            webhook_url,
            json=payload,
            headers={"Content-Type": "application/json"},
            timeout=10
        )
        response.raise_for_status()
        print(f"✅ Discord notification sent successfully!")
        return True
    except requests.exceptions.RequestException as e:
        print(f"❌ Failed to send Discord notification: {e}")
        return False


def send_build_failure_notification(webhook_url, version, error_log):
    """Send build failure notification to Discord"""
    
    embed = {
        "title": "❌ OpenFIDO-ESP Build Failed",
        "description": f"Production build for {version} failed",
        "color": 0xff0000,  # Red
        "fields": [
            {
                "name": "📦 Version",
                "value": version,
                "inline": True
            },
            {
                "name": "📅 Date",
                "value": datetime.utcnow().strftime("%Y-%m-%d %H:%M UTC"),
                "inline": True
            },
            {
                "name": "🔍 Error Log",
                "value": f"```\n{error_log[:500]}\n```" if error_log else "No error log available",
                "inline": False
            }
        ],
        "footer": {
            "text": "OpenFIDO-ESP • Build System"
        },
        "timestamp": datetime.utcnow().isoformat()
    }
    
    payload = {
        "content": "**Build Failed!** ⚠️",
        "embeds": [embed],
        "username": "OpenFIDO-ESP Bot"
    }
    
    try:
        response = requests.post(webhook_url, json=payload, timeout=10)
        response.raise_for_status()
        print(f"✅ Failure notification sent successfully!")
        return True
    except requests.exceptions.RequestException as e:
        print(f"❌ Failed to send failure notification: {e}")
        return False


def main():
    """Main entry point"""
    
    # Get webhook URL from environment
    webhook_url = os.environ.get('DISCORD_WEBHOOK_URL')
    if not webhook_url:
        print("⚠️ DISCORD_WEBHOOK_URL not set, skipping notification")
        sys.exit(0)
    
    # Get notification type
    notification_type = os.environ.get('NOTIFICATION_TYPE', 'release')
    
    if notification_type == 'release':
        version = os.environ.get('VERSION', 'unknown')
        changelog = os.environ.get('CHANGELOG', '')
        download_url = os.environ.get('DOWNLOAD_URL', '')
        commit_sha = os.environ.get('COMMIT_SHA', 'unknown')
        
        success = send_discord_notification(
            webhook_url, version, changelog, download_url, commit_sha
        )
    elif notification_type == 'failure':
        version = os.environ.get('VERSION', 'unknown')
        error_log = os.environ.get('ERROR_LOG', '')
        
        success = send_build_failure_notification(webhook_url, version, error_log)
    else:
        print(f"❌ Unknown notification type: {notification_type}")
        sys.exit(1)
    
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
