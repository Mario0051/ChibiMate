#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QDialog>
#include <QTimer>
#include <QMovie>
#include <QPixmap>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTransform>
#include <QScreen>
#include <QDirIterator>
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QRandomGenerator>
#include <QRect>
#include <QFileInfo>
#include <QObject>

#include <iostream>
#include <vector>

// Class declaration
class TransparentGifViewer;
//class MenuWindow : public QDialog { // Removed the incomplete declaration
//public:
//};

// Function to convert string to lowercase
std::string toLower(std::string str);

// MenuWindow Class Definition
class MenuWindow : public QDialog {
    Q_OBJECT

public:
    MenuWindow(TransparentGifViewer *parent = nullptr);

signals:
    void toggleAutoModeSignal();
    void nextGifSignal();

public slots:
    void update_button_text();

private slots:
    void toggle_auto_mode();
    void next_gif();
    void quit_application();

private:
    TransparentGifViewer *parentViewer;
    QPushButton *auto_mode_btn;
};

// TransparentGifViewer Class Definition
class TransparentGifViewer : public QWidget {
    Q_OBJECT

public:
    TransparentGifViewer();
    bool auto_mode;
    void start_animation();

signals:
    void animation_error(const QString &message);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void next_frame();
    void next_gif();
    void toggle_auto_mode();
    void start_auto_mode();
    void stop_auto_mode();
    void auto_change_state();
    void start_moving();
    void move_window();
    void toggle_menu();
    void show_error(const QString &message);
    void load_current_gif();

private:
    QImage crop_image(const QImage &img);

    QLabel *image_label;
    int current_gif_index;
    int current_frame;
    std::vector<QPixmap> frames;
    std::vector<QPixmap> flipped_frames;
    std::vector<int> durations;
    QTimer *animation_timer;

    bool dragging;
    QPoint drag_position;

    int pre_drag_gif_index;
    bool pre_drag_auto_mode;
    bool pre_drag_moving;
    std::string pre_drag_move_direction;
    bool pre_drag_auto_timer_active;
    int pre_drag_auto_timer_remaining;
    bool pre_drag_move_timer_active;
    int pick_gif_index;

    std::vector<std::string> gif_files;

    QTimer *auto_timer;
    std::string move_direction;
    int move_speed;
    QTimer *move_timer;
    bool moving;

    MenuWindow *menu_window;
    bool menu_visible;
};

// MenuWindow Class Implementation
MenuWindow::MenuWindow(TransparentGifViewer *parent)
    : QDialog(parent), parentViewer(parent)
{
    setWindowTitle("ChibiMate");
    setStyleSheet("background-color: rgba(50, 50, 50, 255);");
    setFixedSize(300, 200);

    // Center relative to the parent window
    if (parentViewer) {
        move(parentViewer->x() + parentViewer->width() / 2 - width() / 2,
             parentViewer->y() + parentViewer->height() / 2 - height() / 2);
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            move(screen->geometry().center() - frameGeometry().center());
        }
    }

    // Layout
    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);

    // Menu title
    QLabel *title = new QLabel("ChibiMate");
    title->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title, 0, 0, 1, 2);

    // Toggle auto mode button
    auto_mode_btn = new QPushButton("Toggle Auto Mode");
    auto_mode_btn->setStyleSheet("color: white; background-color: #444;");
    connect(auto_mode_btn, &QPushButton::clicked, this, &MenuWindow::toggle_auto_mode);
    layout->addWidget(auto_mode_btn, 1, 0, 1, 2);

    // Next GIF button
    QPushButton *next_gif_btn = new QPushButton("Next GIF");
    next_gif_btn->setStyleSheet("color: white; background-color: #444;");
    connect(next_gif_btn, &QPushButton::clicked, this, &MenuWindow::next_gif);
    layout->addWidget(next_gif_btn, 2, 0, 1, 2);

    // Close menu button
    QPushButton *close_menu_btn = new QPushButton("Close Menu");
    close_menu_btn->setStyleSheet("color: white; background-color: #444;");
    connect(close_menu_btn, &QPushButton::clicked, this, &MenuWindow::close);
    layout->addWidget(close_menu_btn, 3, 0);

    // Quit application button
    QPushButton *quit_app_btn = new QPushButton("Quit");
    quit_app_btn->setStyleSheet("color: white; background-color: #700;");
    connect(quit_app_btn, &QPushButton::clicked, this, &MenuWindow::quit_application);
    layout->addWidget(quit_app_btn, 3, 1);

    update_button_text();
}

void MenuWindow::update_button_text()
{
    if (parentViewer) {
        QString status = parentViewer->auto_mode ? "ON" : "OFF";
        auto_mode_btn->setText(QString("Auto Mode: ") + status);
    }
}

void MenuWindow::toggle_auto_mode()
{
    emit toggleAutoModeSignal();
    update_button_text();
}

void MenuWindow::next_gif()
{
    emit nextGifSignal();
}

void MenuWindow::quit_application()
{
    QApplication::quit();
}

// toLower implementation
std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return str;
}

// TransparentGifViewer Class Implementation
TransparentGifViewer::TransparentGifViewer()
    : current_gif_index(0),
    current_frame(0),
    animation_timer(new QTimer(this)),
    dragging(false),
    pre_drag_gif_index(0),
    pre_drag_auto_mode(false),
    pre_drag_moving(false),
    pre_drag_auto_timer_active(false),
    pre_drag_auto_timer_remaining(0),
    pre_drag_move_timer_active(false),
    pick_gif_index(-1),
    auto_mode(false),
    auto_timer(new QTimer(this)),
    move_direction("right"),
    move_speed(5),
    move_timer(new QTimer(this)),
    moving(false),
    menu_window(new MenuWindow(this)),
    menu_visible(false)
{
    setWindowTitle("GIF Viewer");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background-color: rgba(30, 30, 30, 30);");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    image_label = new QLabel(this);
    image_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(image_label);

    setMouseTracking(true);

    connect(animation_timer, &QTimer::timeout, this, &TransparentGifViewer::next_frame);
    connect(this, &TransparentGifViewer::animation_error, this, &TransparentGifViewer::show_error);
    connect(auto_timer, &QTimer::timeout, this, &TransparentGifViewer::auto_change_state);
    connect(move_timer, &QTimer::timeout, this, &TransparentGifViewer::move_window);
    connect(menu_window, &MenuWindow::toggleAutoModeSignal, this, &TransparentGifViewer::toggle_auto_mode);
    connect(menu_window, &MenuWindow::nextGifSignal, this, &TransparentGifViewer::next_gif);

    // Initialize gif_files with file names from the directory
    QDir executableDir = QCoreApplication::applicationDirPath();
    QFileInfoList fileInfoList = executableDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &fileInfo : fileInfoList) {
        if (fileInfo.suffix().toLower() == "webp" || fileInfo.suffix().toLower() == "gif") {
            gif_files.push_back(fileInfo.fileName().toStdString());
        }
    }

    // Find the index of the "pick" GIF
    auto it = std::find_if(gif_files.begin(), gif_files.end(), [](const std::string& s){
        std::string lower_s = toLower(s);
        QString qs = QString::fromStdString(lower_s);
        return qs.contains("pick");
    });
    if (it != gif_files.end()) {
        pick_gif_index = std::distance(gif_files.begin(), it);
    }

    if (!gif_files.empty()) {
        load_current_gif();
        show();
    } else {
        std::cerr << "No GIF files found in the application directory!" << std::endl;
    }
}

QImage TransparentGifViewer::crop_image(const QImage &img)
{
    int width = img.width();
    int height = img.height();
    int crop_top = static_cast<int>(height * 0.3);
    return img.copy(QRect(0, crop_top, width, height - crop_top));
}

void TransparentGifViewer::load_current_gif()
{
    if (gif_files.empty())
        return;

    animation_timer->stop();
    frames.clear();
    flipped_frames.clear();
    durations.clear();
    current_frame = 0;

    QString current_file = QString::fromStdString(gif_files[current_gif_index]);
#ifndef QT_NO_DEBUG
    qDebug() << "Loading:" << current_file;
#endif

    QString file_path = QCoreApplication::applicationDirPath() + "/" + current_file; // Modified line

    QMovie movie(file_path); // Use file_path
    if (!movie.isValid()) {
        emit animation_error(QString("Error loading GIF: ") + current_file);
        return;
    }

    int frame_count = movie.frameCount();
    movie.jumpToFrame(0);

    for (int i = 0; i < frame_count; ++i) {
        QImage current = movie.currentImage();
        if (current.isNull())
            break;

        current = crop_image(current);
        QPixmap pixmap = QPixmap::fromImage(current);

        // Also create flipped version for left movement
        QPixmap flipped_pixmap = pixmap.transformed(QTransform().scale(-1, 1));

        frames.push_back(pixmap);
        flipped_frames.push_back(flipped_pixmap);
        durations.push_back(movie.nextFrameDelay());

        if (i < frame_count - 1)
            movie.jumpToFrame(i + 1);
    }

    if (frames.empty()) {
        emit animation_error(QString("No frames found in ") + current_file);
        return;
    }

    resize(frames[0].width(), frames[0].height());
    image_label->setPixmap(frames[0]);
    image_label->resize(frames[0].size());

    start_animation();
}

void TransparentGifViewer::show_error(const QString &message)
{
    std::cerr << "ERROR: " << message.toStdString() << std::endl;
}

void TransparentGifViewer::start_animation()
{
    if (!frames.empty() && !durations.empty()) {
        animation_timer->start(durations[0]);
    }
}

void TransparentGifViewer::next_frame()
{
    if (frames.empty())
        return;

    current_frame = (current_frame + 1) % frames.size();

    // If in auto mode and moving, the move_window method handles displaying the correct frame
    if (!(auto_mode && moving)) {
        image_label->setPixmap(frames[current_frame]);
    }

    int duration = (current_frame < durations.size()) ? durations[current_frame] : 100;
    animation_timer->setInterval(duration);
}

void TransparentGifViewer::next_gif()
{
    if (!gif_files.empty()) {
        current_gif_index = (current_gif_index + 1) % gif_files.size();
        load_current_gif();
    }
}

void TransparentGifViewer::toggle_auto_mode()
{
    auto_mode = !auto_mode;
#ifndef QT_NO_DEBUG
    qDebug() << "Auto mode:" << (auto_mode ? "ON" : "OFF");
#endif

    if (auto_mode) {
        start_auto_mode();
    } else {
        stop_auto_mode();
    }
    menu_window->update_button_text();
}

void TransparentGifViewer::start_auto_mode()
{
    auto_change_state();
}

void TransparentGifViewer::stop_auto_mode()
{
    auto_timer->stop();
    move_timer->stop();
    moving = false;
}

void TransparentGifViewer::auto_change_state()
{
    // Stop current movement if any
    move_timer->stop();
    moving = false;

    // Choose a random duration between 10-20 seconds
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(10000, 20000);
    auto_timer->start(distrib(gen));

    // 50% chance to move, 50% chance to show random gif
    std::uniform_int_distribution<> bool_distrib(0, 1);
    if (bool_distrib(gen)) {
        start_moving();
    } else {
        if (gif_files.size() > 1) {
            int new_index = current_gif_index;
            std::uniform_int_distribution<> index_distrib(0, gif_files.size() - 1);
            while (new_index == current_gif_index) {
                new_index = index_distrib(gen);
            }
            current_gif_index = new_index;
            load_current_gif();
        }
    }
}

void TransparentGifViewer::start_moving()
{
    moving = true;

    // Find and load the walking animation if available
    int move_gif_index = -1;
    QString lower_filename;
    for (size_t i = 0; i < gif_files.size(); ++i) {
        lower_filename = QString::fromStdString(gif_files[i]).toLower();
        if (lower_filename.contains("move") || lower_filename.contains("walk")) {
            move_gif_index = i;
            break;
        }
    }

    if (move_gif_index != -1) {
        current_gif_index = move_gif_index;
        load_current_gif();
    }

    // Randomly choose direction
    std::uniform_int_distribution<unsigned int> direction_index_distrib(0, 1);
    std::vector<std::string> possibleDirections = {"left", "right"};
    if (direction_index_distrib(*QRandomGenerator::global()) < possibleDirections.size()) {
        move_direction = possibleDirections[direction_index_distrib(*QRandomGenerator::global())];
    } else {
        move_direction = "right";
    }

    // Start the movement timer
    move_timer->start(50);
}

void TransparentGifViewer::move_window()
{
    if (!moving)
        return;

    QPoint pos = this->pos();
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screen_rect = screen->availableGeometry();
    QRect window_rect = this->frameGeometry();

    if (move_direction == "right" && window_rect.right() + move_speed > screen_rect.right()) {
        move_direction = "left";
#ifndef QT_NO_DEBUG
        qDebug() << "Reached right screen edge, reversing direction";
#endif
    } else if (move_direction == "left" && window_rect.left() - move_speed < screen_rect.left()) {
        move_direction = "right";
#ifndef QT_NO_DEBUG
        qDebug() << "Reached left screen edge, reversing direction";
#endif
    }

    if (move_direction == "right") {
        image_label->setPixmap(frames[current_frame]);
        move(pos.x() + move_speed, pos.y());
    } else {
        image_label->setPixmap(flipped_frames[current_frame]);
        move(pos.x() - move_speed, pos.y());
    }
}

void TransparentGifViewer::toggle_menu()
{
    if (menu_visible) {
        menu_window->hide();
        menu_visible = false;
    } else {
        menu_window->update_button_text();
        menu_window->move(x() + width() / 2 - menu_window->width() / 2,
                          y() + height() / 2 - menu_window->height() / 2);
        menu_window->show();
        menu_visible = true;
    }
}

void TransparentGifViewer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        if (!auto_mode) {
            next_gif();
        }
    } else if (event->key() == Qt::Key_A) {
        toggle_auto_mode();
    } else if (event->key() == Qt::Key_M) {
        toggle_menu();
    } else if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void TransparentGifViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        pre_drag_gif_index = current_gif_index;
        pre_drag_auto_mode = auto_mode;
        pre_drag_moving = moving;
        pre_drag_move_direction = move_direction;

        pre_drag_auto_timer_active = auto_timer->isActive();
        if (pre_drag_auto_timer_active) {
            pre_drag_auto_timer_remaining = auto_timer->remainingTime();
        }

        pre_drag_move_timer_active = move_timer->isActive();

        dragging = true;
        drag_position = event->globalPosition().toPoint() - this->frameGeometry().topLeft();

        auto_timer->stop();
        move_timer->stop();
        moving = false;

        if (auto_mode) {
#ifndef QT_NO_DEBUG
            qDebug() << "Auto mode paused for dragging (moving:" << pre_drag_moving << ", direction:" << QString::fromStdString(pre_drag_move_direction) << ")";
#endif
        }

        if (pick_gif_index != -1) {
            current_gif_index = pick_gif_index;
            load_current_gif();
#ifndef QT_NO_DEBUG
            qDebug() << "Switched to 'pick' GIF for dragging";
#endif
        }

        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void TransparentGifViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && dragging) {
        move(event->globalPosition().toPoint() - drag_position);
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void TransparentGifViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (dragging) {
        dragging = false;

        if (current_gif_index == pick_gif_index) {
            current_gif_index = pre_drag_gif_index;
            load_current_gif();
#ifndef QT_NO_DEBUG
            qDebug() << "Restored to previous GIF index" << pre_drag_gif_index;
#endif
        }

        if (pre_drag_auto_mode) {
            auto_mode = true;

            if (pre_drag_moving) {
                moving = true;
                move_direction = pre_drag_move_direction;
                if (pre_drag_move_timer_active) {
                    move_timer->start(50);
#ifndef QT_NO_DEBUG
                    qDebug() << "Restored movement in direction:" << QString::fromStdString(move_direction);
#endif
                }
            }

            if (pre_drag_auto_timer_active) {
                int remaining_time = std::max(pre_drag_auto_timer_remaining, 1000);
                auto_timer->start(remaining_time);
#ifndef QT_NO_DEBUG
                qDebug() << "Restored auto timer with" << remaining_time << "ms remaining";
#endif
            } else {
                start_auto_mode();
            }

#ifndef QT_NO_DEBUG
            qDebug() << "Auto mode state fully restored after dragging";
#endif
        }
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifndef QT_NO_DEBUG
    qDebug() << "Qt 6 is being used for the UI";
    qDebug() << "Using Qt platform plugin:" << qApp->platformName();
#endif

    TransparentGifViewer viewer;

    return a.exec();
}

#include "ChibiViewer.moc"
