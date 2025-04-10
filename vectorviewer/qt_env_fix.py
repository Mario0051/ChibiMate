import os
import sys
import subprocess

def fix_qt_environment():
    """Set environment variables to prevent Qt library conflicts"""
    print("Setting up environment for Qt...")
    
    python_path = sys.executable
    
    env = os.environ.copy()
    
    for var in ['QT_PLUGIN_PATH', 'QT_QPA_PLATFORM_PLUGIN_PATH']:
        if var in env:
            del env[var]
    
    if sys.platform == 'darwin':
        conda_prefix = env.get('CONDA_PREFIX')
        if conda_prefix:
            qt_path = os.path.join(conda_prefix, 'lib', 'python' + sys.version[:3], 
                                  'site-packages', 'PyQt5', 'Qt5')
            if os.path.exists(qt_path):
                env['QT_PLUGIN_PATH'] = os.path.join(qt_path, 'plugins')
                print(f"Set QT_PLUGIN_PATH to {env['QT_PLUGIN_PATH']}")
    
    print("Launching application with fixed environment...")
    script_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'qt_gif_viewer_fixed.py')
    result = subprocess.run([python_path, script_path], env=env)
    return result.returncode

if __name__ == "__main__":
    sys.exit(fix_qt_environment()) 