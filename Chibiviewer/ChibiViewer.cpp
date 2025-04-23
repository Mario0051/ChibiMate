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
#include <QMap>
#include <QVariant>
#include <QPointer>
#include <QCloseEvent>
#include <QListWidget>
#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

class MenuWindow;
class Furniture;

//=============================================================================
// Furniture Class
// Class to represent a furniture item with its properties
//=============================================================================
class Furniture {
public:
    QString filename;
    QPoint use_point; // (x, y) tuple for character positioning
    QString use_type; // "sit" or "laying"
    int x_offset;     // Offset from default x position
    int y_offset;     // Offset from default y position
    QPixmap pixmap;
    QPoint position;  // Current position on screen
    bool visible;
    int width;
    int height;
    QPointer<QWidget> window; // Separate window for the furniture
    QLabel* label;          // Label to display the furniture image

    Furniture(const QString& fname, const QPoint& u_point, const QString& u_type, int x_off = 0, int y_off = 0)
        : filename(fname), use_point(u_point), use_type(u_type), x_offset(x_off), y_offset(y_off),
        visible(false), width(0), height(0), label(nullptr)
    {
        load_image();

        // Create a separate window for the furniture
        window = new QWidget();
        // Set window flags - stay on top but below character
        window->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        window->setAttribute(Qt::WA_TranslucentBackground);
        window->setStyleSheet("background-color: rgba(0, 0, 0, 0);");

        label = new QLabel(window);
        if (!pixmap.isNull()) {
            label->setPixmap(pixmap);
            // Make sure the label is positioned at (0,0) in the window
            label->setGeometry(0, 0, width, height);
            window->resize(width, height);
        } else {
            window->resize(100, 100); // Default size if image fails
        }
    }

    ~Furniture() {
        if (window) {
            window->deleteLater();
        }
    }

    // Load the furniture image
    void load_image() {
        QString image_path = QCoreApplication::applicationDirPath() + "/" + filename;
        if (QFile::exists(image_path)) {
            pixmap.load(image_path);
            if (!pixmap.isNull()) {
                width = pixmap.width();
                height = pixmap.height();
#ifndef QT_NO_DEBUG
                qDebug() << "Loaded furniture:" << filename << "(" << width << "x" << height << ")";
#endif
            }
        } else {
#ifndef QT_NO_DEBUG
            qDebug() << "Furniture image not found:" << image_path;
#endif
        }
    }

    // Set the absolute position of the furniture
    void set_position(int x, int y) {
        position = QPoint(x, y);
        if (window) {
            window->move(x, y);
        }
    }

    // Show the furniture window
    void show() {
        if (window && !pixmap.isNull()) {
            window->show();
            // Raise the window to ensure it's on top of other applications
            window->raise();
            visible = true;
        }
    }

    // Hide the furniture window
    void hide() {
        if (window) {
            window->hide();
            visible = false;
        }
    }

    // Get the absolute position for character use point
    QPoint get_use_position() const {
        return QPoint(position.x() + use_point.x(), position.y() + use_point.y());
    }

    // Get the x center of the furniture
    int get_center_x() const {
        return position.x() + (width / 2);
    }
};

//=============================================================================
// TransparentGifViewer Class
// A transparent window that displays GIF animations
//=============================================================================
class TransparentGifViewer : public QWidget {
    Q_OBJECT

public:
    TransparentGifViewer();
    ~TransparentGifViewer() override;
    bool auto_mode; // Public for MenuWindow access
    void start_animation();

    void create_furniture(const QString& furniture_type);
    void start_walk_to_furniture();

    QStringList get_available_prefixes() const;
    QString get_current_prefix() const;

signals:
    void animation_error(const QString &message);

public slots:
    void set_character_prefix(const QString& prefix);

protected:
    // Handle key press events
    void keyPressEvent(QKeyEvent *event) override;
    // Handle mouse press events for dragging
    void mousePressEvent(QMouseEvent *event) override;
    // Handle mouse move events for dragging
    void mouseMoveEvent(QMouseEvent *event) override;
    // Handle mouse release events
    void mouseReleaseEvent(QMouseEvent *event) override;
    // Handle window close event
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Display the next frame in the animation
    void next_frame();
    // Toggle between automatic and manual mode
    void toggle_auto_mode();
    // Start automatic mode behavior
    void start_auto_mode();
    // Stop automatic mode behavior
    void stop_auto_mode();
    // Randomly choose next behavior in auto mode
    void auto_change_state();
    // Start moving the window
    void start_moving();
    // Move the window in the current direction
    void move_window();
    // Use the current furniture
    void use_furniture();
    // Called when the furniture use timer expires
    void finish_using_furniture();
    // Toggle the menu window visibility
    void toggle_menu();
    // Display error message
    void show_error(const QString &message);
    // Load and display the current GIF
    void load_current_gif();
    // Handle add furniture signal from menu
    void handle_add_furniture(const QString& type);
    // Load the next GIF in the list
    void next_gif();

private:
    // Crop the top 30% of the image and shift up
    QImage crop_image(const QImage &img);
    // Find all GIF and WebP files and identify character prefixes
    void discover_animations_and_prefixes();
    // Filter animations for the selected prefix and load the default state
    void filter_and_load_gifs(const QString& prefix);
    // Categorize GIFs by their type based on filename
    void categorize_gifs();
    // Ensure character window is always above furniture window
    void raise_window_above_furniture();
    // Helper to extract prefix and action from filename
    QString extract_prefix_and_action(const QString& filename, QString& action_found);

    // UI Elements
    QLabel *image_label;

    // Animation state variables
    int current_gif_index;
    int current_frame;
    std::vector<QPixmap> frames;
    std::vector<QPixmap> flipped_frames; // Also create flipped version for left movement
    std::vector<int> durations;
    QTimer *animation_timer;

    // Dragging state
    bool dragging;
    QPoint drag_position;

    // Variables to store state during dragging
    int pre_drag_gif_index;
    bool pre_drag_auto_mode;
    bool pre_drag_moving;
    QString pre_drag_move_direction;
    QString pre_drag_facing_direction;
    QString pre_drag_behavior;
    bool pre_drag_auto_timer_active;
    int pick_gif_index; // Index of the "pick" GIF

    // Character/Prefix Management
    QString current_prefix;
    QStringList available_raw_prefixes;
    QMap<QString, QString> all_animation_files;
    std::vector<std::string> gif_files; // Store filtered gif/webp filenames for current prefix
    QMap<QString, int> gif_type_indices; // Dictionary mapping gif types to their indices
    const QStringList known_actions = {"layingalt", "laying", "sit", "walk", "wait", "pick", "move", "lay"};

    // New variables for automatic mode
    QTimer *auto_timer;
    QString current_behavior; // wait, walk, walk_to_furniture, use_furniture
    QString move_direction; // Direction of movement
    QString facing_direction; // Direction character is facing (may differ from movement)
    int move_speed;
    QTimer *move_timer;
    bool moving;

    // Furniture variables
    Furniture* current_furniture;
    QMap<QString, QMap<QString, QVariant>> furniture_types; // Add furniture types dictionary to store available furniture
    int furniture_target_x;
    QTimer *furniture_use_timer; // Add furniture state timer for minimum duration
    bool walked_to_furniture; // Track whether the character just walked to furniture

    // Menu
    QPointer<MenuWindow> menu_window; // Create menu window (but don't show it yet)
    bool menu_visible;
};


//=============================================================================
// MenuWindow Class
// Menu window with options to control the application
//=============================================================================
class MenuWindow : public QDialog {
    Q_OBJECT

public:
    MenuWindow(TransparentGifViewer *parent = nullptr);
    // Update button text based on current state
    void update_button_text();
    void update_prefix_list_selection();

signals:
    void toggleAutoModeSignal();
    void addFurnitureSignal(const QString& type);
    void prefixSelectedSignal(const QString& prefix);

private slots:
    // Toggle auto mode from the menu
    void toggle_auto_mode();
    // Add furniture to the scene using the specified furniture type
    void add_couch();
    void add_table();
    // Quit the application
    void quit_application();
    void on_prefix_selected();

private:
    // Helper to capitalize prefix for display (with special case)
    QString capitalize(const QString& str);

    TransparentGifViewer *parentViewer; // Keep track of the parent
    QPushButton *auto_mode_btn;
    QListWidget *prefix_list_widget;
};

// --- MenuWindow Implementation ---
MenuWindow::MenuWindow(TransparentGifViewer *parent)
    : QDialog(parent), parentViewer(parent)
{
    setWindowTitle("ChibiMate");
    setStyleSheet("background-color: rgba(50, 50, 50, 255); color: white;");
    setMinimumSize(320, 400);

    // Layout
    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);

    // Menu title
    QLabel *title = new QLabel("ChibiMate");
    title->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title, 0, 0, 1, 2);

    // Character Selection
    QLabel *prefix_label = new QLabel("Select Character:");
    prefix_label->setStyleSheet("color: white; margin-top: 10px;");
    layout->addWidget(prefix_label, 1, 0, 1, 2);

    prefix_list_widget = new QListWidget(this);
    prefix_list_widget->setStyleSheet("background-color: #333; color: white; border: 1px solid #555;");
    layout->addWidget(prefix_list_widget, 2, 0, 1, 2);

    // Populate the list
    if (parentViewer) {
        QStringList raw_prefixes = parentViewer->get_available_prefixes();
        for (const QString& raw_prefix : raw_prefixes) {
            if (raw_prefix.isEmpty()) continue;
            QString display_name = capitalize(raw_prefix);
            QListWidgetItem *item = new QListWidgetItem(display_name);
            item->setData(Qt::UserRole, raw_prefix);
            prefix_list_widget->addItem(item);
        }
        update_prefix_list_selection();
    }
    // Connect selection change signal
    connect(prefix_list_widget, &QListWidget::currentItemChanged, this, &MenuWindow::on_prefix_selected);

    // Toggle auto mode button
    auto_mode_btn = new QPushButton("Toggle Auto Mode");
    auto_mode_btn->setStyleSheet("color: white; background-color: #444; margin-top: 10px;");
    connect(auto_mode_btn, &QPushButton::clicked, this, &MenuWindow::toggle_auto_mode);
    layout->addWidget(auto_mode_btn, 3, 0, 1, 2);

    // Furniture label
    QLabel* furniture_label = new QLabel("Add Furniture:");
    furniture_label->setStyleSheet("color: white; margin-top: 10px;");
    layout->addWidget(furniture_label, 4, 0, 1, 2);

    // Add couch button
    QPushButton *add_couch_btn = new QPushButton("Add Couch");
    add_couch_btn->setStyleSheet("color: white; background-color: #444;");
    connect(add_couch_btn, &QPushButton::clicked, this, &MenuWindow::add_couch);
    layout->addWidget(add_couch_btn, 5, 0);

    // Add table button
    QPushButton *add_table_btn = new QPushButton("Add Table");
    add_table_btn->setStyleSheet("color: white; background-color: #444;");
    connect(add_table_btn, &QPushButton::clicked, this, &MenuWindow::add_table);
    layout->addWidget(add_table_btn, 5, 1);

    // Close menu button
    QPushButton *close_menu_btn = new QPushButton("Close Menu");
    close_menu_btn->setStyleSheet("color: white; background-color: #444; margin-top: 15px;");
    connect(close_menu_btn, &QPushButton::clicked, this, &MenuWindow::close);
    layout->addWidget(close_menu_btn, 6, 0);

    // Quit application button
    QPushButton *quit_app_btn = new QPushButton("Quit");
    quit_app_btn->setStyleSheet("color: white; background-color: #700; margin-top: 15px;");
    connect(quit_app_btn, &QPushButton::clicked, this, &MenuWindow::quit_application);
    layout->addWidget(quit_app_btn, 6, 1);

    // Adjust size and position
    layout->activate();
    adjustSize();
    if (parentViewer) {
        // Center relative to the parent window
        move(parentViewer->x() + parentViewer->width() / 2 - width() / 2,
             parentViewer->y() + parentViewer->height() / 2 - height() / 2);
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            move(screen->geometry().center() - frameGeometry().center());
        }
    }
    update_button_text();
}

void MenuWindow::update_button_text() {
    if (parentViewer) {
        QString status = parentViewer->auto_mode ? "ON" : "OFF";
        auto_mode_btn->setText(QString("Auto Mode: ") + status);
    } else {
        auto_mode_btn->setText("Auto Mode: ???");
    }
}

void MenuWindow::update_prefix_list_selection() {
    if (!parentViewer || !prefix_list_widget) return;
    QString current_raw_prefix = parentViewer->get_current_prefix();
    for (int i = 0; i < prefix_list_widget->count(); ++i) {
        QListWidgetItem *item = prefix_list_widget->item(i);
        if (item->data(Qt::UserRole).toString() == current_raw_prefix) {
            bool old_signals_state = prefix_list_widget->blockSignals(true);
            prefix_list_widget->setCurrentItem(item);
            prefix_list_widget->blockSignals(old_signals_state);
            break;
        }
    }
}

void MenuWindow::toggle_auto_mode() {
    emit toggleAutoModeSignal();
    update_button_text();
}

void MenuWindow::add_couch() {
    emit addFurnitureSignal("couch");
    close();
}

void MenuWindow::add_table() {
    emit addFurnitureSignal("table");
    close();
}

void MenuWindow::quit_application() {
    QApplication::quit();
}

void MenuWindow::on_prefix_selected() {
    QListWidgetItem *current = prefix_list_widget->currentItem();
    if (current && parentViewer) {
        QString raw_prefix = current->data(Qt::UserRole).toString();
        if (raw_prefix != parentViewer->get_current_prefix()) {
            emit prefixSelectedSignal(raw_prefix);
        }
    }
}

QString MenuWindow::capitalize(const QString& str) {
    if (str.isEmpty()) return str;
    if (str.toLower() == "hk416") {
        return "HK416";
    }
    return str.at(0).toUpper() + str.mid(1).toLower();
}


//=============================================================================
// TransparentGifViewer Implementation
//=============================================================================

TransparentGifViewer::TransparentGifViewer()
    : current_gif_index(0),
    current_frame(0),
    animation_timer(new QTimer(this)),
    dragging(false),
    pre_drag_gif_index(0),
    pre_drag_auto_mode(false),
    pre_drag_moving(false),
    pre_drag_move_direction("right"),
    pre_drag_facing_direction("right"),
    pre_drag_behavior("wait"),
    pre_drag_auto_timer_active(false),
    current_prefix(""),
    auto_mode(false),
    auto_timer(new QTimer(this)),
    current_behavior("wait"),
    move_direction("right"),
    facing_direction("right"),
    move_speed(5),
    move_timer(new QTimer(this)),
    moving(false),
    current_furniture(nullptr),
    furniture_target_x(0),
    furniture_use_timer(new QTimer(this)),
    walked_to_furniture(false),
    menu_visible(false),
    pick_gif_index(-1)
{
    setWindowTitle("ChibiMate");
    // Ensure the character window is always on top
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background-color: rgba(30, 30, 30, 30);");

    // Main layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    image_label = new QLabel(this);
    image_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(image_label);
    setMouseTracking(true);

    connect(animation_timer, &QTimer::timeout, this, &TransparentGifViewer::next_frame);
    connect(auto_timer, &QTimer::timeout, this, &TransparentGifViewer::auto_change_state);
    connect(move_timer, &QTimer::timeout, this, &TransparentGifViewer::move_window);
    connect(furniture_use_timer, &QTimer::timeout, this, &TransparentGifViewer::finish_using_furniture);
    furniture_use_timer->setSingleShot(true);
    connect(this, &TransparentGifViewer::animation_error, this, &TransparentGifViewer::show_error);

    // Define available furniture types
    QMap<QString, QVariant> couch_props;
    couch_props["filename"] = "couch.png";
    couch_props["use_point_x"] = 100; couch_props["use_point_y"] = 200;
    couch_props["use_type"] = "sit"; couch_props["x_offset"] = 0; couch_props["y_offset"] = -70;
    furniture_types["couch"] = couch_props;
    QMap<QString, QVariant> table_props;
    table_props["filename"] = "table.png";
    table_props["use_point_x"] = 50; table_props["use_point_y"] = 150;
    table_props["use_type"] = "sit"; table_props["x_offset"] = 0; table_props["y_offset"] = -50;
    furniture_types["table"] = table_props;

    // Load the GIFs and categorize them
    discover_animations_and_prefixes();

    QString initial_prefix = "";
    if (!available_raw_prefixes.isEmpty()) {
        initial_prefix = available_raw_prefixes.first();
    }

    // Create menu window
    menu_window = new MenuWindow(this);
    connect(menu_window, &MenuWindow::toggleAutoModeSignal, this, &TransparentGifViewer::toggle_auto_mode);
    connect(menu_window, &MenuWindow::addFurnitureSignal, this, &TransparentGifViewer::handle_add_furniture);
    connect(menu_window, &MenuWindow::prefixSelectedSignal, this, &TransparentGifViewer::set_character_prefix);
    connect(menu_window, &QDialog::finished, this, [this](){ menu_visible = false; });

    // Load the initial character
    if (!initial_prefix.isEmpty()) {
        filter_and_load_gifs(initial_prefix);
        show();
    } else {
        // Handle case where no character could be loaded
        QLabel *errorLabel = new QLabel("Error: No character animations found.\nPlease add files like 'prefixAction.gif'\n(e.g., 'charaWait.gif') to the application directory.", this);
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLabel->setStyleSheet("background-color: rgba(50,0,0,200); color: white; padding: 20px; font-size: 14px;");
        layout->addWidget(errorLabel);
        image_label->hide();
        setFixedSize(400, 150);
        setStyleSheet("background-color: rgba(50,0,0,180);");
        show();
    }
}

TransparentGifViewer::~TransparentGifViewer() {
    if (current_furniture) {
        delete current_furniture;
        current_furniture = nullptr;
    }
}

void TransparentGifViewer::closeEvent(QCloseEvent *event) {
    // Make sure to clean up furniture window when main window closes
    if (current_furniture && current_furniture->window) {
        current_furniture->window->close();
    }
    if (menu_window && menu_window->isVisible()) {
        menu_window->close();
    }
    QWidget::closeEvent(event);
}

QString TransparentGifViewer::extract_prefix_and_action(const QString& filename, QString& action_found) {
    QFileInfo fileInfo(filename);
    QString baseNameLower = fileInfo.completeBaseName().toLower();
    action_found = "";
    QString prefix = "";

    for (const QString& action : known_actions) {
        if (baseNameLower.endsWith(action)) {
            action_found = action;
            prefix = baseNameLower.left(baseNameLower.length() - action.length());
            return prefix;
        }
    }
    return "";
}


void TransparentGifViewer::discover_animations_and_prefixes() {
    all_animation_files.clear();
    available_raw_prefixes.clear();
    std::set<QString> unique_prefixes;

    QDir appDir(QCoreApplication::applicationDirPath());
    QDirIterator it(appDir.path(), QStringList() << "*.gif" << "*.webp", QDir::Files, QDirIterator::NoIteratorFlags);

    int file_count = 0;
    while (it.hasNext()) {
        it.next();
        file_count++;
        QString filename = it.fileName();
        QString action_found;
        QString prefix = extract_prefix_and_action(filename, action_found);

        if (!prefix.isEmpty() && !action_found.isEmpty()) {
            all_animation_files[filename] = prefix;
            unique_prefixes.insert(prefix);
        }
    }

    available_raw_prefixes = QStringList(unique_prefixes.begin(), unique_prefixes.end());
    available_raw_prefixes.sort();
#ifndef QT_NO_DEBUG
    qDebug() << "Found" << file_count << "animation files.";
#endif
}

void TransparentGifViewer::filter_and_load_gifs(const QString& prefix_to_load) {
#ifndef QT_NO_DEBUG
    qDebug() << "Loading prefix:" << prefix_to_load;
#endif

    if (prefix_to_load == current_prefix && !gif_files.empty()) {
        return;
    }

    current_prefix = prefix_to_load;
    gif_files.clear();

    for (auto it = all_animation_files.constBegin(); it != all_animation_files.constEnd(); ++it) {
        if (it.value() == prefix_to_load) {
            gif_files.push_back(it.key().toStdString());
        }
    }

    if (gif_files.empty()) {
        emit animation_error("No animations found for selected character: " + prefix_to_load);
        image_label->clear();
        resize(100,100);
        return;
    }

    if (current_furniture) {
        finish_using_furniture();
    }
    stop_auto_mode();
    if (menu_window) menu_window->update_button_text();
    moving = false;
    walked_to_furniture = false;
    facing_direction = "right";

    categorize_gifs();

    current_behavior = "wait";
    current_gif_index = gif_type_indices.value("wait", -1);
    if (current_gif_index < 0 || current_gif_index >= static_cast<int>(gif_files.size())) {
        current_gif_index = 0;
    }

    if (gif_files.empty()) {
        emit animation_error("Cannot load initial GIF for " + current_prefix);
        image_label->clear();
        resize(100,100);
        return;
    }

    load_current_gif();

    if (menu_window && menu_window->isVisible()) {
        menu_window->update_prefix_list_selection();
    }
}

void TransparentGifViewer::categorize_gifs() {
    gif_type_indices.clear();
    gif_type_indices["wait"] = -1; gif_type_indices["walk"] = -1; gif_type_indices["sit"] = -1;
    gif_type_indices["laying"] = -1; gif_type_indices["pick"] = -1;
    gif_type_indices["move"] = -1; gif_type_indices["lay"] = -1; gif_type_indices["layingalt"] = -1;

    pick_gif_index = -1;

    for (size_t i = 0; i < gif_files.size(); ++i) {
        QString filename = QString::fromStdString(gif_files[i]);
        QString action_found;
        extract_prefix_and_action(filename, action_found);

        if (!action_found.isEmpty()) {
            bool categorized = false;
            if (gif_type_indices.contains(action_found) && gif_type_indices[action_found] == -1) {
                gif_type_indices[action_found] = static_cast<int>(i);
                categorized = true;
                if(action_found == "pick") pick_gif_index = static_cast<int>(i);
            }
#ifndef QT_NO_DEBUG
            if(categorized) qDebug() << "Found" << action_found << "GIF:" << filename;
#endif
        }
    }

    if (gif_type_indices["walk"] == -1 && gif_type_indices["move"] != -1) {
        gif_type_indices["walk"] = gif_type_indices["move"];
    }
    if (gif_type_indices["laying"] == -1) {
        if (gif_type_indices["lay"] != -1) {
            gif_type_indices["laying"] = gif_type_indices["lay"];
        } else if (gif_type_indices["layingalt"] != -1) {
            gif_type_indices["laying"] = gif_type_indices["layingalt"];
        }
    }

    int fallback_wait_index = 0;
    if (gif_type_indices["wait"] == -1) {
        if (!gif_files.empty()) {
            gif_type_indices["wait"] = fallback_wait_index;
        }
    }

    int wait_index = gif_type_indices.value("wait", -1);
    if (wait_index != -1) {
        if (gif_type_indices["walk"] == -1) { gif_type_indices["walk"] = wait_index; }
        if (gif_type_indices["sit"] == -1) { gif_type_indices["sit"] = wait_index; }
        if (gif_type_indices["laying"] == -1) { gif_type_indices["laying"] = wait_index; }
        if (pick_gif_index != -1) { gif_type_indices["pick"] = pick_gif_index; }
    }

    gif_type_indices.remove("move");
    gif_type_indices.remove("lay");
    gif_type_indices.remove("layingalt");
}


QStringList TransparentGifViewer::get_available_prefixes() const {
    return available_raw_prefixes;
}

QString TransparentGifViewer::get_current_prefix() const {
    return current_prefix;
}

void TransparentGifViewer::set_character_prefix(const QString& prefix) {
    if (!available_raw_prefixes.contains(prefix)) {
        return;
    }
    filter_and_load_gifs(prefix);
}

QImage TransparentGifViewer::crop_image(const QImage &img) {
    if (img.isNull()) return img;
    int width = img.width();
    int height = img.height();
    int crop_top = static_cast<int>(height * 0.3);
    if (crop_top >= height || crop_top < 0) return QImage();
    return img.copy(0, crop_top, width, height - crop_top);
}

void TransparentGifViewer::load_current_gif() {
    if (gif_files.empty()) {
        emit animation_error("No GIFs available for " + current_prefix);
        image_label->clear(); resize(100,100); return;
    }
    if (current_gif_index < 0 || current_gif_index >= static_cast<int>(gif_files.size())) {
        current_gif_index = gif_type_indices.value("wait", 0);
        if (current_gif_index < 0 || current_gif_index >= static_cast<int>(gif_files.size())) {
            current_gif_index = 0;
        }
        if (current_gif_index < 0 || current_gif_index >= static_cast<int>(gif_files.size())) {
            emit animation_error("Cannot load any GIF index for " + current_prefix);
            image_label->clear(); resize(100,100); return;
        }
    }

    animation_timer->stop();
    frames.clear();
    flipped_frames.clear();
    durations.clear();
    current_frame = 0;

    QString current_file = QString::fromStdString(gif_files[current_gif_index]);
#ifndef QT_NO_DEBUG
    qDebug() << "Loading:" << current_file;
#endif


    QString file_path = QCoreApplication::applicationDirPath() + "/" + current_file;
    QMovie movie(file_path);

    if (!movie.isValid()) {
        emit animation_error(QString("Error loading GIF (invalid movie): ") + current_file);
        image_label->clear(); resize(100,100); return;
    }

    int frame_count = movie.frameCount();
    int loaded_frame_count = 0;
    if (frame_count <= 0) {
        QPixmap pixmap = movie.currentPixmap();
        if (!pixmap.isNull()) {
            QImage current = pixmap.toImage();
            current = crop_image(current);
            if (!current.isNull()) {
                frames.push_back(QPixmap::fromImage(current));
                flipped_frames.push_back(frames[0].transformed(QTransform().scale(-1, 1)));
                durations.push_back(1000);
                loaded_frame_count = 1;
            } else { emit animation_error(QString("Error cropping static image: ") + current_file); image_label->clear(); resize(100,100); return; }
        } else { emit animation_error(QString("No frames or invalid image in: ") + current_file); image_label->clear(); resize(100,100); return; }
    } else {
        movie.jumpToFrame(0);
        for (int i = 0; i < frame_count; ++i) {
            QImage current = movie.currentImage();
            if (current.isNull()) {
                if (i < frame_count - 1 && !movie.jumpToNextFrame()) break;
                continue;
            }
            QImage cropped = crop_image(current);
            if (cropped.isNull()) {
                if (i < frame_count - 1 && !movie.jumpToNextFrame()) break;
                continue;
            }
            QPixmap pixmap = QPixmap::fromImage(cropped);
            QPixmap flipped_pixmap = pixmap.transformed(QTransform().scale(-1, 1));
            frames.push_back(pixmap);
            flipped_frames.push_back(flipped_pixmap);
            durations.push_back(movie.nextFrameDelay());
            loaded_frame_count++;

            if (i < frame_count - 1) {
                if (!movie.jumpToNextFrame()) break;
            }
        }
    }


    if (frames.empty()) {
        emit animation_error(QString("No valid frames extracted from ") + current_file);
        image_label->clear(); resize(100,100); return;
    }

    resize(frames[0].width(), frames[0].height());
    image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);
    image_label->resize(frames[0].size());
    image_label->setGeometry(0, 0, width(), height());

    start_animation();
}


void TransparentGifViewer::show_error(const QString &message) {
    qWarning() << "ERROR:" << message;
}

void TransparentGifViewer::start_animation() {
    if (!frames.empty() && !durations.empty() && current_frame < durations.size() && current_frame >= 0) {
        int duration = durations[current_frame] > 0 ? durations[current_frame] : 100;
        animation_timer->start(duration);
    } else if (!frames.empty()) {
        image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);
        animation_timer->stop();
    }
}

void TransparentGifViewer::next_frame() {
    if (frames.empty()) {
        animation_timer->stop();
        return;
    }
    if (frames.size() == 1) {
        if(animation_timer->isActive()) animation_timer->stop();
        return;
    }

    current_frame = (current_frame + 1) % frames.size();

    if (current_frame < frames.size() && current_frame < durations.size()) {
        image_label->setPixmap(facing_direction == "right" ? frames[current_frame] : flipped_frames[current_frame]);
        int duration = durations[current_frame] > 0 ? durations[current_frame] : 100;
        animation_timer->setInterval(duration);
        if (!animation_timer->isActive() && duration > 0) {
            animation_timer->start();
        } else if (duration <= 0 && animation_timer->isActive()) {
            animation_timer->setInterval(100);
            if(!animation_timer->isActive()) animation_timer->start();
        }
    } else {
        current_frame = 0;
        if (!frames.empty()) {
            image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);
        }
        animation_timer->setInterval(100);
        if (!animation_timer->isActive()) animation_timer->start();
    }
}

void TransparentGifViewer::next_gif() {
    if (gif_files.empty()) return;
    int current_raw_index = current_gif_index;
    int next_raw_index = (current_raw_index + 1) % gif_files.size();
    current_gif_index = next_raw_index;
    load_current_gif();
}

void TransparentGifViewer::toggle_auto_mode() {
    auto_mode = !auto_mode;
    if(menu_window) menu_window->update_button_text();
#ifndef QT_NO_DEBUG
    qDebug() << "Auto mode:" << (auto_mode ? "ON" : "OFF");
#endif
    if (auto_mode) {
        start_auto_mode();
    } else {
        stop_auto_mode();
    }
}

void TransparentGifViewer::start_auto_mode() {
    auto_change_state();
}

void TransparentGifViewer::stop_auto_mode() {
    auto_timer->stop();
    move_timer->stop();
    furniture_use_timer->stop();

    moving = false;
    current_behavior = "wait";
    int wait_index = gif_type_indices.value("wait", -1);
    if (wait_index != -1 && wait_index < static_cast<int>(gif_files.size())) {
        current_gif_index = wait_index;
        load_current_gif();
    } else if (!gif_files.empty()) {
        current_gif_index = 0;
        load_current_gif();
    }
}

void TransparentGifViewer::auto_change_state() {
    if (!auto_mode) return;
    move_timer->stop();
    moving = false;

    QStringList possible_behaviors;
    possible_behaviors << "wait" << "walk";
    if (!current_furniture) possible_behaviors << "walk_to_furniture";
    else if (walked_to_furniture) { possible_behaviors.clear(); possible_behaviors << "use_furniture"; }

    if (possible_behaviors.size() > 1) {
        if (current_behavior == "wait") possible_behaviors.removeAll("wait");
        else if (current_behavior == "walk" || current_behavior == "walk_to_furniture") possible_behaviors.removeAll("walk");
        else if (current_behavior == "sit" || current_behavior == "laying") possible_behaviors.removeAll("use_furniture");
    }
    if (possible_behaviors.isEmpty()) possible_behaviors << "wait";

    QString next_logical_behavior = possible_behaviors.at(QRandomGenerator::global()->bounded(possible_behaviors.size()));
#ifndef QT_NO_DEBUG
    qDebug() << "Auto mode: Switching to behavior:" << next_logical_behavior;
#endif

    if(next_logical_behavior != "use_furniture") walked_to_furniture = false;

    int index = -1;
    if (next_logical_behavior == "wait") {
        current_behavior = "wait";
        index = gif_type_indices.value("wait", -1);
        if (index != -1 && index < static_cast<int>(gif_files.size())) { current_gif_index = index; load_current_gif(); }
        else if (!gif_files.empty()) { current_gif_index = 0; load_current_gif(); }
    } else if (next_logical_behavior == "walk") {
        start_moving();
    } else if (next_logical_behavior == "walk_to_furniture") {
        QStringList available_types = furniture_types.keys();
        if (!available_types.isEmpty()) create_furniture(available_types.at(QRandomGenerator::global()->bounded(available_types.size())));
        else { current_behavior = "wait"; index = gif_type_indices.value("wait", 0); if (!gif_files.empty() && index < static_cast<int>(gif_files.size()) && index >=0) {current_gif_index = index; load_current_gif();} else if (!gif_files.empty()) { current_gif_index = 0; load_current_gif(); } }
    } else if (next_logical_behavior == "use_furniture") {
        use_furniture();
    }

    std::uniform_int_distribution<> distrib(10000, 20000);
    int duration = distrib(*QRandomGenerator::global());
    if (current_behavior != "sit" && current_behavior != "laying") {
        auto_timer->start(duration);
    } else {
        auto_timer->stop();
        if (!furniture_use_timer->isActive()) furniture_use_timer->start(duration);
    }
}

void TransparentGifViewer::start_moving() {
    moving = true;
    current_behavior = "walk";
    int index = gif_type_indices.value("walk", -1);

    if (index != -1 && index < static_cast<int>(gif_files.size())) {
        current_gif_index = index;
        load_current_gif();
    }

    move_direction = (QRandomGenerator::global()->bounded(2) == 0) ? "left" : "right";
    facing_direction = move_direction;
    move_timer->start(50);
}

void TransparentGifViewer::start_walk_to_furniture() {
    if (!current_furniture) return;
    moving = true;
    walked_to_furniture = false;
    current_behavior = "walk_to_furniture";
    int index = gif_type_indices.value("walk", -1);

    if (index != -1 && index < static_cast<int>(gif_files.size())) {
        current_gif_index = index;
        load_current_gif();
    }

    int character_center_x = x() + width() / 2;
    furniture_target_x = current_furniture->get_center_x();
    move_direction = (furniture_target_x > character_center_x) ? "right" : "left";
    facing_direction = move_direction;
#ifndef QT_NO_DEBUG
    qDebug() << "Starting walk to furniture in" << move_direction << "direction";
#endif
    move_timer->start(50);
}


void TransparentGifViewer::move_window() {
    if (!moving) return;
    QPoint current_pos = pos(); int current_x = current_pos.x(); int next_x = current_x;
    if (move_direction == "right") next_x = current_x + move_speed; else next_x = current_x - move_speed;

    if (current_behavior == "walk") {
        QScreen *screen = QGuiApplication::screenAt(current_pos); if (!screen) screen = QGuiApplication::primaryScreen(); QRect screen_rect = screen->availableGeometry();
        if (move_direction == "right" && (next_x + width()) > screen_rect.right()) {
            next_x = screen_rect.right() - width(); move_direction = "left"; facing_direction = "left";
#ifndef QT_NO_DEBUG
            qDebug() << "Reached right screen edge, reversing direction";
#endif
        }
        else if (move_direction == "left" && next_x < screen_rect.left()) {
            next_x = screen_rect.left(); move_direction = "right"; facing_direction = "right";
#ifndef QT_NO_DEBUG
            qDebug() << "Reached left screen edge, reversing direction";
#endif
        }
    }

    if (!frames.empty() && current_frame >= 0 && current_frame < frames.size()) {
        image_label->setPixmap(facing_direction == "right" ? frames[current_frame] : flipped_frames[current_frame]);
    } else if (!frames.empty()) {
        image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);
    }

    move(next_x, current_pos.y());

    if (current_behavior == "walk_to_furniture" && current_furniture) {
        int character_center_x = next_x + width() / 2;
        if (std::abs(character_center_x - furniture_target_x) < move_speed * 2) {
#ifndef QT_NO_DEBUG
            qDebug() << "Reached furniture, switching to use_furniture";
#endif
            walked_to_furniture = true; moving = false; move_timer->stop(); use_furniture();
        }
    }
    if (current_furniture && current_furniture->visible) raise_window_above_furniture();
}

void TransparentGifViewer::create_furniture(const QString& furniture_type) {
    if (!furniture_types.contains(furniture_type)) { return; }
    if (current_furniture) { delete current_furniture; current_furniture = nullptr; }

    QMap<QString, QVariant> props = furniture_types[furniture_type];
    QString filename = props["filename"].toString();
    QPoint use_point(props["use_point_x"].toInt(), props["use_point_y"].toInt());
    QString use_type = props["use_type"].toString();
    int x_offset = props["x_offset"].toInt(); int y_offset = props["y_offset"].toInt();
    current_furniture = new Furniture(filename, use_point, use_type, x_offset, y_offset);

    if (current_furniture->pixmap.isNull()) { delete current_furniture; current_furniture = nullptr; return; }

    QScreen *screen = QGuiApplication::screenAt(pos()); if (!screen) screen = QGuiApplication::primaryScreen(); QRect screen_rect = screen->availableGeometry();
    int furniture_width = current_furniture->width; int furniture_height = current_furniture->height; int margin = 20;
    int min_x = screen_rect.left() + margin; int max_x = screen_rect.right() - furniture_width - margin;
    int base_furniture_x = (max_x >= min_x) ? QRandomGenerator::global()->bounded(min_x, max_x + 1) : min_x;
    int char_bottom = y() + height();
    int base_furniture_y = char_bottom - furniture_height + y_offset;

    int final_x = base_furniture_x + x_offset;
    int final_y = base_furniture_y;

    final_x = std::max(min_x, std::min(final_x, max_x));
    final_y = std::max(screen_rect.top() + margin, std::min(base_furniture_y, screen_rect.bottom() - furniture_height - margin));

    current_furniture->set_position(final_x, final_y);
    current_furniture->show();
    QTimer::singleShot(20, this, &TransparentGifViewer::raise_window_above_furniture);
#ifndef QT_NO_DEBUG
    qDebug() << "Created" << furniture_type << "at (" << final_x << "," << final_y << ") with offsets (" << x_offset << "," << y_offset << ")";
#endif

    furniture_target_x = current_furniture->get_center_x(); walked_to_furniture = false;
    if (auto_mode) start_walk_to_furniture();
}

void TransparentGifViewer::use_furniture() {
    if (!current_furniture) return;
    moving = false; move_timer->stop(); current_behavior = "use_furniture";
    QPoint use_pos = current_furniture->get_use_position(); QString use_type = current_furniture->use_type;
    int char_x = use_pos.x() - width() / 2; int char_y = use_pos.y() - height(); move(char_x, char_y);

    int index = -1; QString final_behavior = "wait";
    if (use_type == "sit") { final_behavior = "sit"; index = gif_type_indices.value("sit", -1); }
    else if (use_type == "laying") { final_behavior = "laying"; index = gif_type_indices.value("laying", -1); }
    else { index = gif_type_indices.value("wait", -1); }
    current_behavior = final_behavior;

    if (index != -1 && index < static_cast<int>(gif_files.size())) { current_gif_index = index; load_current_gif(); }
    else if (!gif_files.empty()) { current_gif_index = 0; load_current_gif(); }

    int furniture_center_x = current_furniture->get_center_x();
    facing_direction = (furniture_center_x >= char_x + width() / 2) ? "right" : "left";
    raise_window_above_furniture();

    if (auto_mode) {
#ifndef QT_NO_DEBUG
        qDebug() << "Using furniture in auto mode - will end with next state change";
#endif
        if (!furniture_use_timer->isActive()){ std::uniform_int_distribution<> d(10000, 20000); furniture_use_timer->start(d(*QRandomGenerator::global())); }
    } else {
        furniture_use_timer->stop();
#ifndef QT_NO_DEBUG
        qDebug() << "Using furniture in manual mode - press spacebar to change";
#endif
    }
    walked_to_furniture = false;
}

void TransparentGifViewer::finish_using_furniture() {
#ifndef QT_NO_DEBUG
    qDebug() << "Finished using furniture, removing it";
#endif
    if (current_furniture) { current_furniture->hide(); delete current_furniture; current_furniture = nullptr; }
    walked_to_furniture = false; moving = false; move_timer->stop();
    current_behavior = "wait";
    int index = gif_type_indices.value("wait", -1);
    if (index != -1 && index < static_cast<int>(gif_files.size())) { current_gif_index = index; load_current_gif(); }
    else if (!gif_files.empty()) {current_gif_index = 0; load_current_gif();}

    if (auto_mode) QTimer::singleShot(100, this, &TransparentGifViewer::auto_change_state);
}

void TransparentGifViewer::toggle_menu() {
    if (menu_visible) {
        if (menu_window) menu_window->hide();
        menu_visible = false;
    } else {
        if (!menu_window) menu_window = new MenuWindow(this);
        if (menu_window) {
            menu_window->update_button_text();
            menu_window->update_prefix_list_selection();
            menu_window->adjustSize();
            menu_window->move(x() + width() / 2 - menu_window->width() / 2, y() + height() / 2 - menu_window->height() / 2);
            menu_window->show();
            menu_window->raise();
            menu_visible = true;
        }
    }
}

void TransparentGifViewer::raise_window_above_furniture() {
    this->raise();
}

void TransparentGifViewer::handle_add_furniture(const QString& type) {
    create_furniture(type);
}

void TransparentGifViewer::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space && !auto_mode) { if (current_behavior == "sit" || current_behavior == "laying") finish_using_furniture(); else next_gif(); }
    else if (event->key() == Qt::Key_A) toggle_auto_mode();
    else if (event->key() == Qt::Key_M) toggle_menu();
    else if (event->key() == Qt::Key_F && !auto_mode) { create_furniture("couch"); if(current_furniture) start_walk_to_furniture(); }
    else if (event->key() == Qt::Key_T && !auto_mode) { if (furniture_types.contains("table")) { create_furniture("table"); if(current_furniture) start_walk_to_furniture(); } }
    else if (event->key() == Qt::Key_Escape) close();
    else QWidget::keyPressEvent(event);
}

void TransparentGifViewer::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Save complete current state
        pre_drag_gif_index = current_gif_index; pre_drag_auto_mode = auto_mode; pre_drag_moving = moving;
        pre_drag_move_direction = move_direction; pre_drag_facing_direction = facing_direction;
        pre_drag_behavior = current_behavior; pre_drag_auto_timer_active = auto_timer->isActive();

        // Delete furniture if it exists
        if (current_furniture) {
#ifndef QT_NO_DEBUG
            qDebug() << "Removing furniture during pick";
#endif
            current_furniture->hide(); delete current_furniture; current_furniture = nullptr; walked_to_furniture = false;
        }
        // Pause all current activity
        auto_timer->stop(); move_timer->stop(); furniture_use_timer->stop(); moving = false;
        dragging = true; drag_position = event->globalPosition().toPoint() - this->frameGeometry().topLeft();

        // Switch to the "pick" GIF if available
        if (pick_gif_index != -1 && pick_gif_index < static_cast<int>(gif_files.size())) {
            current_behavior = "pick"; current_gif_index = pick_gif_index; load_current_gif();
#ifndef QT_NO_DEBUG
            qDebug() << "Switched to 'pick' GIF for dragging";
#endif
        } else { current_behavior = "pick"; }
        facing_direction = pre_drag_facing_direction;
        if (!frames.empty() && current_frame >=0 && current_frame < frames.size()) image_label->setPixmap(facing_direction == "right" ? frames[current_frame] : flipped_frames[current_frame]);
        else if (!frames.empty()) image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);
        event->accept();
    } else QWidget::mousePressEvent(event);
}

void TransparentGifViewer::mouseMoveEvent(QMouseEvent *event) {
    if (dragging && (event->buttons() & Qt::LeftButton)) { move(event->globalPosition().toPoint() - drag_position); event->accept(); }
    else QWidget::mouseMoveEvent(event);
}

void TransparentGifViewer::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && dragging) {
        dragging = false;
// Default to wait state after pick
#ifndef QT_NO_DEBUG
        qDebug() << "Defaulting to wait state after pick";
#endif
        current_behavior = "wait";
        int wait_index = gif_type_indices.value("wait", -1);
        if (wait_index != -1 && wait_index < static_cast<int>(gif_files.size())) { current_gif_index = wait_index; load_current_gif(); }
        else if (!gif_files.empty()){ current_gif_index = 0; load_current_gif();}

        facing_direction = pre_drag_facing_direction;
        if (!frames.empty() && current_frame >=0 && current_frame < frames.size()) image_label->setPixmap(facing_direction == "right" ? frames[current_frame] : flipped_frames[current_frame]);
        else if (!frames.empty()) image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);

        // Restore auto mode if it was active
        if (pre_drag_auto_mode) {
            auto_mode = true;
#ifndef QT_NO_DEBUG
            qDebug() << "Auto mode restored after dragging";
#endif
            QTimer::singleShot(10, this, &TransparentGifViewer::auto_change_state);
        } else {
            auto_mode = false; auto_timer->stop(); move_timer->stop(); furniture_use_timer->stop(); moving = false;
        }
        event->accept();
    } else QWidget::mouseReleaseEvent(event);
}


//=============================================================================
// Main Function
//=============================================================================
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
#ifndef QT_NO_DEBUG
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Platform Plugin:" << qApp->platformName();
    qDebug() << "Application Dir:" << QCoreApplication::applicationDirPath();
#endif
    TransparentGifViewer viewer;
    return a.exec();
}

// MOC include must be LAST
#include "ChibiViewer.moc" // Ensure this filename matches
