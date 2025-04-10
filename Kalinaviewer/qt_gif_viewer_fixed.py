#!/usr/bin/env python3
"""
Qt GIF Viewer with Advanced Transparency
---------------------------------------
This application displays GIF animations in a frameless, transparent window.
It uses PyQt5 specifically for better compatibility across environments.
"""

import os
import sys
import glob
from PIL import Image, ImageSequence

if 'QT_QPA_PLATFORM_PLUGIN_PATH' in os.environ:
    del os.environ['QT_QPA_PLATFORM_PLUGIN_PATH']

try:
    from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout
    from PyQt5.QtCore import Qt, QTimer, QSize, QPoint, pyqtSignal
    from PyQt5.QtGui import QPixmap, QImage, QKeyEvent, QMouseEvent
except ImportError:
    print("PyQt5 is required but not installed.")
    print("Install it with: pip install PyQt5")
    sys.exit(1)

class TransparentGifViewer(QWidget):
    """A transparent window that displays GIF animations"""
    
    animation_error = pyqtSignal(str)
    
    def __init__(self):
        super(TransparentGifViewer, self).__init__()
        
        self.setWindowTitle("GIF Viewer")
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setStyleSheet("background-color: rgba(30, 30, 30, 30)")
        
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)
        
        self.image_label = QLabel(self)
        self.image_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.image_label)
        
        self.current_gif_index = 0
        self.current_frame = 0
        self.frames = []
        self.durations = []
        self.animation_timer = QTimer(self)
        self.animation_timer.timeout.connect(self.next_frame)
        
        self.dragging = False
        self.drag_position = None
        self.setMouseTracking(True)
        
        self.animation_error.connect(self.show_error)
        
        self.gif_files = []
        self.load_gif_files()
        
        if self.gif_files:
            self.load_current_gif()
            self.show()
        else:
            print("No GIF or WebP files found in the current directory!")
            
    def load_gif_files(self):
        """Find all GIF and WebP files in the current directory"""
        self.gif_files = glob.glob("*.gif") + glob.glob("*.webp")
        print(f"Found {len(self.gif_files)} animation files: {self.gif_files}")
    
    def crop_image(self, img):
        """Crop the top 30% of the image and shift up"""
        width, height = img.size
        crop_top = int(height * 0.3)
        return img.crop((0, crop_top, width, height))
    
    def pil_to_qt_image(self, pil_image):
        """Convert PIL Image to QImage"""
        if pil_image.mode != "RGBA":
            pil_image = pil_image.convert("RGBA")
            
        data = pil_image.tobytes("raw", "RGBA")
        return QImage(data, pil_image.width, pil_image.height, QImage.Format_RGBA8888)
    
    def load_current_gif(self):
        """Load and display the current GIF"""
        if not self.gif_files:
            return
            
        self.animation_timer.stop()
        
        current_file = self.gif_files[self.current_gif_index]
        print(f"Loading: {current_file}")
        
        try:
            img = Image.open(current_file)
            
            self.frames = []
            self.durations = []
            
            frame_count = 0
            for frame in ImageSequence.Iterator(img):
                frame_count += 1
            
            img.seek(0)
            
            for i in range(frame_count):
                try:
                    current = img.copy()
                    
                    current = self.crop_image(current)
                    
                    qt_image = self.pil_to_qt_image(current)
                    pixmap = QPixmap.fromImage(qt_image)
                    
                    self.frames.append(pixmap)
                    self.durations.append(img.info.get('duration', 100))
                    
                    if i < frame_count - 1:
                        img.seek(i + 1)
                except EOFError:
                    break
            
            if not self.frames:
                self.animation_error.emit(f"No frames found in {current_file}")
                return
                
            self.resize(self.frames[0].width(), self.frames[0].height())
            
            self.current_frame = 0
            self.image_label.setPixmap(self.frames[0])
            self.image_label.resize(self.frames[0].size())
            
            self.start_animation()
            
        except Exception as e:
            self.animation_error.emit(f"Error loading GIF: {str(e)}")
            import traceback
            traceback.print_exc()
    
    def show_error(self, message):
        """Display error message"""
        print(f"ERROR: {message}")
    
    def start_animation(self):
        """Start the animation timer"""
        if self.frames and self.durations:
            self.animation_timer.start(self.durations[0])
            
    def next_frame(self):
        """Display the next frame in the animation"""
        if not self.frames:
            return
            
        self.current_frame = (self.current_frame + 1) % len(self.frames)
        
        self.image_label.setPixmap(self.frames[self.current_frame])
        
        duration = self.durations[self.current_frame] if self.current_frame < len(self.durations) else 100
        self.animation_timer.setInterval(duration)
    
    def next_gif(self):
        """Load the next GIF in the list"""
        if self.gif_files:
            self.current_gif_index = (self.current_gif_index + 1) % len(self.gif_files)
            self.load_current_gif()
    
    def keyPressEvent(self, event):
        """Handle key press events"""
        if event.key() == Qt.Key_Space:
            self.next_gif()
        elif event.key() == Qt.Key_Escape:
            self.close()
        else:
            super().keyPressEvent(event)
    
    def mousePressEvent(self, event):
        """Handle mouse press events for dragging"""
        if event.button() == Qt.LeftButton:
            self.dragging = True
            self.drag_position = event.globalPos() - self.frameGeometry().topLeft()
            event.accept()
    
    def mouseMoveEvent(self, event):
        """Handle mouse move events for dragging"""
        if event.buttons() & Qt.LeftButton and self.dragging:
            self.move(event.globalPos() - self.drag_position)
            event.accept()
    
    def mouseReleaseEvent(self, event):
        """Handle mouse release events"""
        self.dragging = False

if __name__ == "__main__":
    app = QApplication(sys.argv)
    
    print(f"Python version: {sys.version}")
    print(f"PyQt5 is being used for the UI")
    print(f"Using Qt platform plugin: {app.platformName()}")
    
    viewer = TransparentGifViewer()
    
    sys.exit(app.exec_()) 