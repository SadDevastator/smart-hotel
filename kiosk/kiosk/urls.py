from django.urls import path
from . import views

app_name = 'kiosk'

urlpatterns = [
    # Main kiosk flow
    path('', views.advertisement, name='advertisement'),
    path('language/', views.choose_language, name='choose_language'),
    path('checkin/', views.checkin, name='checkin'),
    path('passport/', views.start, name='start'),
    path('upload-scan/', views.upload_scan, name='upload_scan'),
    path('extract/status/<int:task_id>/', views.extract_status, name='extract_status'),
    path('verify/', views.verify_info, name='verify_info'),
    
    # DW Registration Card (DW R.C.) routes
    path('dw-registration/', views.dw_registration_card, name='dw_registration_card'),
    path('dw-registration/sign/', views.dw_sign_document, name='dw_sign_document'),
    path('dw-registration/print/', views.dw_generate_pdf, name='dw_generate_pdf'),
    
    # Legacy document routes (kept for compatibility)
    path('document/', views.documentation, name='documentation'),
    path('document/sign/', views.document_signing, name='document_signing'),
    
    # Reservation and access
    path('reservation/', views.reservation_entry, name='reservation_entry'),
    path('choose-access/<int:reservation_id>/', views.choose_access, name='choose_access'),
    path('enroll-face/<int:reservation_id>/', views.enroll_face, name='enroll_face'),
    path('final/<int:reservation_id>/', views.finalize, name='finalize'),
    path('submit-keycards/<int:reservation_id>/', views.submit_keycards, name='submit_keycards'),
    
    # API endpoints
    path('api/save-passport-data/', views.save_passport_extraction, name='save_passport_extraction'),
    
    # MRZ Service API proxy endpoints
    path('api/mrz/health/', views.mrz_service_health, name='mrz_service_health'),
    path('api/mrz/video-feed-url/', views.mrz_video_feed_url, name='mrz_video_feed_url'),
    path('api/mrz/capture/', views.mrz_capture, name='mrz_capture'),
    path('api/mrz/start-camera/', views.mrz_start_camera, name='mrz_start_camera'),
    path('api/mrz/stop-camera/', views.mrz_stop_camera, name='mrz_stop_camera'),
    path('api/mrz/detection-status/', views.mrz_detection_status, name='mrz_detection_status'),
]
