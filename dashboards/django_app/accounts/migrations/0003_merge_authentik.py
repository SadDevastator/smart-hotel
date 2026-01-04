# Generated migration to merge conflicting 0002 migrations

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('accounts', '0002_authentik_integration'),
        ('accounts', '0002_passwordresettoken'),
    ]

    operations = [
        # Merge migration - no operations needed
        # The 0002_passwordresettoken creates PasswordResetToken
        # The 0002_authentik_integration deletes it and adds oidc_sub
        # Net result: oidc_sub field exists, PasswordResetToken does not
    ]
