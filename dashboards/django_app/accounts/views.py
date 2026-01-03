from django.contrib.auth import views as auth_views, update_session_auth_hash
from django.contrib.auth.mixins import LoginRequiredMixin, UserPassesTestMixin
from django.contrib.auth.forms import PasswordChangeForm, SetPasswordForm
from django.views.generic import TemplateView, FormView, View, ListView
from django.urls import reverse_lazy, reverse
from django.shortcuts import redirect, get_object_or_404
from django.contrib import messages
from django.http import JsonResponse
from django import forms
import secrets
import string

from .models import User, PasswordResetToken
from dashboard.telegram import send_telegram_message


class AdminRequiredMixin(UserPassesTestMixin):
    """Only allow admin users"""
    def test_func(self):
        return self.request.user.is_authenticated and self.request.user.role == User.ROLE_ADMIN


class AdminOrMonitorRequiredMixin(UserPassesTestMixin):
    """Only allow admin or monitor users"""
    def test_func(self):
        return self.request.user.is_authenticated and self.request.user.role in [User.ROLE_ADMIN, User.ROLE_MONITOR]


class LoginView(auth_views.LoginView):
    template_name = 'accounts/login.html'
    redirect_authenticated_user = True
    
    def form_valid(self, form):
        user = form.get_user()
        if user.is_expired:
            messages.error(self.request, 'Your account has expired. Please contact the front desk.')
            return redirect('accounts:login')
        return super().form_valid(form)
    
    def get_success_url(self):
        user = self.request.user
        if user.is_guest and user.assigned_room:
            return reverse_lazy('dashboard:room_detail', kwargs={'room_id': user.assigned_room.id})
        return reverse_lazy('dashboard:index')


class LogoutView(auth_views.LogoutView):
    next_page = 'accounts:login'


class ProfileView(LoginRequiredMixin, TemplateView):
    template_name = 'accounts/profile.html'
    
    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        context['user'] = self.request.user
        return context


class SettingsView(LoginRequiredMixin, TemplateView):
    """User settings page - change password and theme"""
    template_name = 'accounts/settings.html'
    
    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        context['user'] = self.request.user
        context['can_change_password'] = self.request.user.role in [User.ROLE_ADMIN, User.ROLE_MONITOR]
        return context


class ChangePasswordView(LoginRequiredMixin, AdminOrMonitorRequiredMixin, FormView):
    """Change password for admin and monitor users"""
    template_name = 'accounts/change_password.html'
    form_class = PasswordChangeForm
    success_url = reverse_lazy('accounts:settings')
    
    def get_form_kwargs(self):
        kwargs = super().get_form_kwargs()
        kwargs['user'] = self.request.user
        return kwargs
    
    def form_valid(self, form):
        user = form.save()
        update_session_auth_hash(self.request, user)
        messages.success(self.request, 'Your password has been changed successfully.')
        return super().form_valid(form)


class ForgotPasswordForm(forms.Form):
    username = forms.CharField(max_length=150, widget=forms.TextInput(attrs={
        'class': 'form-input',
        'placeholder': 'Enter your username'
    }))


class ForgotPasswordView(FormView):
    """Request password reset - sends link via Telegram (guests only)"""
    template_name = 'accounts/forgot_password.html'
    form_class = ForgotPasswordForm
    success_url = reverse_lazy('accounts:forgot_password_done')
    
    def form_valid(self, form):
        username = form.cleaned_data['username']
        try:
            user = User.objects.get(username=username)
            
            # Only allow reset for guest users - admin/monitor must be reset manually
            if user.role in [User.ROLE_ADMIN, User.ROLE_MONITOR]:
                messages.error(self.request, 'Staff accounts cannot be reset via this form. Please contact the system administrator.')
                return redirect('accounts:forgot_password')
            
            # Create reset token
            reset_token = PasswordResetToken.create_for_user(user)
            
            # Build reset URL
            reset_url = self.request.build_absolute_uri(
                reverse('accounts:reset_password', kwargs={'token': reset_token.token})
            )
            
            # Send via Telegram
            message = (
                f"<b>Smart Hotel - Password Reset</b>\n\n"
                f"A password reset was requested for user: <b>{user.username}</b>\n\n"
                f"Click the link below to reset your password:\n"
                f"{reset_url}\n\n"
                f"This link will expire in 1 hour.\n\n"
                f"If you did not request this, please ignore this message."
            )
            
            send_telegram_message(message)
            
        except User.DoesNotExist:
            # Don't reveal if user exists or not
            pass
        
        return super().form_valid(form)


class ForgotPasswordDoneView(TemplateView):
    """Confirmation that reset link was sent"""
    template_name = 'accounts/forgot_password_done.html'


class ResetPasswordForm(forms.Form):
    new_password1 = forms.CharField(
        label='New Password',
        widget=forms.PasswordInput(attrs={'class': 'form-input', 'placeholder': 'New password'})
    )
    new_password2 = forms.CharField(
        label='Confirm Password',
        widget=forms.PasswordInput(attrs={'class': 'form-input', 'placeholder': 'Confirm new password'})
    )
    
    def clean(self):
        cleaned_data = super().clean()
        password1 = cleaned_data.get('new_password1')
        password2 = cleaned_data.get('new_password2')
        
        if password1 and password2 and password1 != password2:
            raise forms.ValidationError('Passwords do not match.')
        
        if password1 and len(password1) < 8:
            raise forms.ValidationError('Password must be at least 8 characters.')
        
        return cleaned_data


class ResetPasswordView(FormView):
    """Reset password using token from Telegram link"""
    template_name = 'accounts/reset_password.html'
    form_class = ResetPasswordForm
    success_url = reverse_lazy('accounts:reset_password_done')
    
    def dispatch(self, request, *args, **kwargs):
        token = kwargs.get('token')
        try:
            self.reset_token = PasswordResetToken.objects.get(token=token)
            if not self.reset_token.is_valid:
                messages.error(request, 'This password reset link has expired or already been used.')
                return redirect('accounts:forgot_password')
        except PasswordResetToken.DoesNotExist:
            messages.error(request, 'Invalid password reset link.')
            return redirect('accounts:forgot_password')
        
        return super().dispatch(request, *args, **kwargs)
    
    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        context['username'] = self.reset_token.user.username
        return context
    
    def form_valid(self, form):
        password = form.cleaned_data['new_password1']
        
        user = self.reset_token.user
        user.set_password(password)
        user.save()
        
        # Mark token as used
        self.reset_token.used = True
        self.reset_token.save()
        
        # Notify via Telegram
        message = (
            f"<b>Smart Hotel - Password Changed</b>\n\n"
            f"The password for user <b>{user.username}</b> has been successfully reset.\n\n"
            f"If you did not make this change, please contact the administrator immediately."
        )
        send_telegram_message(message)
        
        messages.success(self.request, 'Your password has been reset successfully. You can now log in.')
        return super().form_valid(form)


class ResetPasswordDoneView(TemplateView):
    """Confirmation that password was reset"""
    template_name = 'accounts/reset_password_done.html'


# ===== Staff Management Views (Admin Only) =====

class StaffManagementView(LoginRequiredMixin, AdminRequiredMixin, ListView):
    """List and manage staff accounts"""
    template_name = 'accounts/staff_management.html'
    context_object_name = 'staff_users'
    
    def get_queryset(self):
        return User.objects.filter(
            role__in=[User.ROLE_ADMIN, User.ROLE_MONITOR]
        ).order_by('-date_joined')
    
    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        context['role_choices'] = [
            (User.ROLE_ADMIN, 'Administrator'),
            (User.ROLE_MONITOR, 'Monitor'),
        ]
        return context


class CreateStaffForm(forms.Form):
    username = forms.CharField(
        max_length=150,
        widget=forms.TextInput(attrs={'class': 'form-input', 'placeholder': 'Username'})
    )
    email = forms.EmailField(
        required=False,
        widget=forms.EmailInput(attrs={'class': 'form-input', 'placeholder': 'Email (optional)'})
    )
    role = forms.ChoiceField(
        choices=[(User.ROLE_ADMIN, 'Administrator'), (User.ROLE_MONITOR, 'Monitor')],
        widget=forms.Select(attrs={'class': 'form-input'})
    )
    
    def clean_username(self):
        username = self.cleaned_data['username']
        if User.objects.filter(username=username).exists():
            raise forms.ValidationError('This username is already taken.')
        return username


class CreateStaffView(LoginRequiredMixin, AdminRequiredMixin, FormView):
    """Create a new staff account"""
    template_name = 'accounts/create_staff.html'
    form_class = CreateStaffForm
    success_url = reverse_lazy('accounts:staff_management')
    
    def form_valid(self, form):
        # Generate a random password
        password = ''.join(secrets.choice(string.ascii_letters + string.digits + '!@#$%') for _ in range(12))
        
        user = User.objects.create_user(
            username=form.cleaned_data['username'],
            email=form.cleaned_data.get('email', ''),
            password=password,
            role=form.cleaned_data['role'],
            created_by=self.request.user
        )
        
        # Send credentials via Telegram
        message = (
            f"<b>Smart Hotel - New Staff Account Created</b>\n\n"
            f"Role: <b>{user.get_role_display()}</b>\n"
            f"Username: <code>{user.username}</code>\n"
            f"Password: <code>{password}</code>\n\n"
            f"Please change this password after first login."
        )
        send_telegram_message(message)
        
        messages.success(
            self.request, 
            f'Staff account "{user.username}" created successfully. Credentials sent via Telegram.'
        )
        return super().form_valid(form)


class AdminResetPasswordView(LoginRequiredMixin, AdminRequiredMixin, View):
    """Admin can reset any staff user's password"""
    
    def post(self, request, user_id):
        try:
            user = get_object_or_404(User, pk=user_id)
            
            # Cannot reset own password this way
            if user == request.user:
                return JsonResponse({'error': 'Use the change password page for your own account.'}, status=400)
            
            # Cannot reset guest passwords this way
            if user.role == User.ROLE_GUEST:
                return JsonResponse({'error': 'Guest passwords should be reset via the guest management page.'}, status=400)
            
            # Generate new password
            new_password = ''.join(secrets.choice(string.ascii_letters + string.digits + '!@#$%') for _ in range(12))
            user.set_password(new_password)
            user.save()
            
            # Send via Telegram
            message = (
                f"<b>Smart Hotel - Password Reset by Admin</b>\n\n"
                f"Account: <b>{user.username}</b>\n"
                f"New Password: <code>{new_password}</code>\n\n"
                f"Reset by: {request.user.username}\n"
                f"Please change this password after login."
            )
            send_telegram_message(message)
            
            return JsonResponse({
                'status': 'success',
                'message': f'Password reset for {user.username}. New credentials sent via Telegram.'
            })
            
        except Exception as e:
            return JsonResponse({'error': str(e)}, status=400)


class DeleteStaffView(LoginRequiredMixin, AdminRequiredMixin, View):
    """Delete a staff account"""
    
    def post(self, request, user_id):
        try:
            user = get_object_or_404(User, pk=user_id)
            
            # Cannot delete own account
            if user == request.user:
                return JsonResponse({'error': 'You cannot delete your own account.'}, status=400)
            
            # Cannot delete superusers
            if user.is_superuser:
                return JsonResponse({'error': 'Superuser accounts cannot be deleted.'}, status=400)
            
            username = user.username
            user.delete()
            
            return JsonResponse({
                'status': 'success',
                'message': f'Account "{username}" has been deleted.'
            })
            
        except Exception as e:
            return JsonResponse({'error': str(e)}, status=400)
