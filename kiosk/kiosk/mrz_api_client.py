"""
MRZ API Client
Communicates with the MRZ microservice for passport scanning and extraction.
"""

import logging
import os
import requests
from typing import Optional
from django.conf import settings

logger = logging.getLogger(__name__)

# Default MRZ service URL - can be overridden via environment variable
MRZ_SERVICE_URL = os.environ.get('MRZ_SERVICE_URL', 'http://mrz-service:5000')


class MRZAPIError(Exception):
    """Raised when MRZ API request fails"""
    def __init__(self, message, error_code=None, details=None):
        self.message = message
        self.error_code = error_code
        self.details = details or {}
        super().__init__(self.message)


class MRZAPIClient:
    """
    Client for communicating with the MRZ microservice.
    
    The MRZ service handles:
    - Camera capture and preview
    - Document detection
    - MRZ extraction
    - Document filling
    """
    
    def __init__(self, base_url: str = None, timeout: int = 30):
        """
        Initialize MRZ API client.
        
        Args:
            base_url: Base URL of the MRZ service. Defaults to MRZ_SERVICE_URL env var.
            timeout: Request timeout in seconds.
        """
        self.base_url = (base_url or MRZ_SERVICE_URL).rstrip('/')
        self.timeout = timeout
        self.session = requests.Session()
        logger.info(f"MRZ API Client initialized with base URL: {self.base_url}")
    
    def health_check(self) -> bool:
        """
        Check if the MRZ service is healthy.
        
        Returns:
            bool: True if service is healthy, False otherwise.
        """
        try:
            response = self.session.get(
                f"{self.base_url}/health",
                timeout=5
            )
            return response.status_code == 200
        except requests.RequestException as e:
            logger.warning(f"MRZ service health check failed: {e}")
            return False
    
    def start_camera(self) -> dict:
        """
        Initialize the camera on the MRZ service.
        
        Returns:
            dict: Response with success status.
        """
        try:
            response = self.session.post(
                f"{self.base_url}/start_camera",
                timeout=self.timeout
            )
            response.raise_for_status()
            return response.json()
        except requests.RequestException as e:
            logger.error(f"Failed to start camera: {e}")
            raise MRZAPIError(f"Failed to start camera: {e}")
    
    def stop_camera(self) -> dict:
        """
        Stop the camera on the MRZ service.
        
        Returns:
            dict: Response with success status.
        """
        try:
            response = self.session.post(
                f"{self.base_url}/stop_camera",
                timeout=self.timeout
            )
            response.raise_for_status()
            return response.json()
        except requests.RequestException as e:
            logger.error(f"Failed to stop camera: {e}")
            raise MRZAPIError(f"Failed to stop camera: {e}")
    
    def get_video_feed_url(self) -> str:
        """
        Get the URL for the video feed stream.
        
        Returns:
            str: URL for the video feed.
        """
        return f"{self.base_url}/video_feed"
    
    def get_detection_status(self) -> dict:
        """
        Get current document detection status.
        
        Returns:
            dict: Detection status with 'detected', 'area_percentage', etc.
        """
        try:
            response = self.session.get(
                f"{self.base_url}/detection_status",
                timeout=self.timeout
            )
            response.raise_for_status()
            return response.json()
        except requests.RequestException as e:
            logger.error(f"Failed to get detection status: {e}")
            raise MRZAPIError(f"Failed to get detection status: {e}")
    
    def capture_and_extract(self) -> dict:
        """
        Capture passport image and extract MRZ data.
        
        Returns:
            dict: Extracted MRZ data including:
                - success: bool
                - data: dict with MRZ fields (surname, given_name, etc.)
                - image_path: str path to saved image
                - timestamp: str timestamp of capture
                - filled_document: dict with document info (if available)
        
        Raises:
            MRZAPIError: If extraction fails.
        """
        try:
            response = self.session.post(
                f"{self.base_url}/capture",
                timeout=self.timeout
            )
            response.raise_for_status()
            result = response.json()
            
            if not result.get('success'):
                error_msg = result.get('error', 'Unknown error')
                error_code = result.get('error_code')
                details = result.get('details', {})
                raise MRZAPIError(error_msg, error_code, details)
            
            return result
        except requests.RequestException as e:
            logger.error(f"Failed to capture and extract: {e}")
            raise MRZAPIError(f"Failed to capture and extract: {e}")
    
    def extract_from_image(self, image_data: bytes, filename: str = "passport.jpg") -> dict:
        """
        Extract MRZ data from an uploaded image.
        
        Args:
            image_data: Raw image bytes.
            filename: Original filename.
        
        Returns:
            dict: Extracted MRZ data.
        
        Raises:
            MRZAPIError: If extraction fails.
        """
        try:
            files = {'image': (filename, image_data, 'image/jpeg')}
            response = self.session.post(
                f"{self.base_url}/api/extract",
                files=files,
                timeout=self.timeout
            )
            response.raise_for_status()
            result = response.json()
            
            if not result.get('success'):
                error_msg = result.get('error', 'Unknown error')
                error_code = result.get('error_code')
                details = result.get('details', {})
                raise MRZAPIError(error_msg, error_code, details)
            
            return result
        except requests.RequestException as e:
            logger.error(f"Failed to extract from image: {e}")
            raise MRZAPIError(f"Failed to extract from image: {e}")
    
    def extract_from_file(self, file_path: str) -> dict:
        """
        Extract MRZ data from a local file.
        
        Args:
            file_path: Path to the image file.
        
        Returns:
            dict: Extracted MRZ data.
        
        Raises:
            MRZAPIError: If extraction fails.
        """
        with open(file_path, 'rb') as f:
            image_data = f.read()
        filename = os.path.basename(file_path)
        return self.extract_from_image(image_data, filename)


# Singleton instance
_mrz_client: Optional[MRZAPIClient] = None


def get_mrz_client() -> MRZAPIClient:
    """
    Get the singleton MRZ API client instance.
    
    Returns:
        MRZAPIClient: The client instance.
    """
    global _mrz_client
    if _mrz_client is None:
        _mrz_client = MRZAPIClient()
    return _mrz_client


def convert_mrz_to_kiosk_format(mrz_data: dict) -> dict:
    """
    Convert MRZ API response data to kiosk format.
    
    Args:
        mrz_data: Raw MRZ data from API.
    
    Returns:
        dict: Kiosk-formatted data with fields:
            - first_name
            - last_name
            - passport_number
            - date_of_birth
            - nationality
            - gender
    """
    # Handle date format conversion
    dob = mrz_data.get('birth_date', '')
    if len(dob) == 6:  # YYMMDD format
        year = int(dob[:2])
        # Assume 00-30 is 2000s, 31-99 is 1900s
        century = 20 if year <= 30 else 19
        dob = f"{century}{dob[:2]}-{dob[2:4]}-{dob[4:6]}"
    
    return {
        'first_name': mrz_data.get('given_name', '').replace('<', ' ').strip(),
        'last_name': mrz_data.get('surname', '').replace('<', ' ').strip(),
        'passport_number': mrz_data.get('document_number', ''),
        'date_of_birth': dob,
        'nationality': mrz_data.get('nationality_code', ''),
        'gender': mrz_data.get('sex', ''),
    }
