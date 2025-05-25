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
#include <QPainter>
#include <QPainterPath>
#include <QElapsedTimer>
#include <QShortcut>

#include <qobjectdefs.h>

#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

class MenuWindow;
class Furniture;
class TransparentGifViewer;

#ifdef slots
#undef slots
#endif

extern "C" {
#include <spine/spine.h>
}

#define slots Q_SLOTS

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
    QPointer<QWidget> window;
    QLabel* label;

    Furniture(const QString& fname, const QPoint& u_point, const QString& u_type, int x_off = 0, int y_off = 0)
        : filename(fname), use_point(u_point), use_type(u_type), x_offset(x_off), y_offset(y_off),
        visible(false), width(0), height(0), label(nullptr) {
        load_image();
        // Create a separate window for the furniture
        window = new QWidget();
        // Set window flags for furniture - make it stay on top but still below character
        window->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        window->setAttribute(Qt::WA_TranslucentBackground);
        window->setStyleSheet("background-color: rgba(0, 0, 0, 0);");
        // Create a label to display the furniture image
        label = new QLabel(window);
        if (!pixmap.isNull()) {
            label->setPixmap(pixmap);
            // Make sure the label is positioned at (0,0) in the window
            label->setGeometry(0, 0, width, height);
            window->resize(width, height);
        } else {
            window->resize(100, 100);
        }
    }

    ~Furniture() {
        if (window) {
            window->deleteLater();
        }
    }

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

    void set_position(int x, int y) {
        position = QPoint(x, y);
        if (window) {
            window->move(x, y);
        }
    }

    void show() {
        if (window && !pixmap.isNull()) {
            // Show the window
            window->show();
            // Raise the window to ensure it's on top of other applications
            window->raise();
            visible = true;
        }
    }

    void hide() {
        if (window) {
            window->hide();
            visible = false;
        }
    }

    QPoint get_use_position() const {
        return QPoint(position.x() + use_point.x(), position.y() + use_point.y());
    }

    int get_center_x() const {
        return position.x() + (width / 2);
    }
};

class TransparentGifViewer : public QWidget {
    Q_OBJECT

public:
    TransparentGifViewer();
    ~TransparentGifViewer() override;

    bool auto_mode;
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
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void next_character_frame_slot();
    void toggle_auto_mode();
    void start_auto_mode();
    void stop_auto_mode();
    void auto_change_state();
    void start_moving();
    void move_window();
    void use_furniture();
    void finish_using_furniture();
    void toggle_menu();
    void show_error(const QString &message);
    void handle_add_furniture(const QString& type);
    void next_gif();

private:
    enum CharacterAssetType {
        TYPE_GIF,
        TYPE_SPINE,
        TYPE_NONE
    };
    CharacterAssetType currentAssetType;

    QLabel *image_label;

    // Animation state variables
    int current_gif_index;
    int current_frame;
    std::vector<QPixmap> frames;
    std::vector<QPixmap> flipped_frames;
    std::vector<int> durations;
    QTimer *animation_timer;

    struct SpineInstance {
        spAtlas* atlas;
        spSkeletonData* skeletonData;
        spSkeleton* skeleton;
        spAnimationStateData* animationStateData;
        spAnimationState* animationState;
        QPixmap currentFramePixmap;
        QMap<QString, spAnimation*> animations;
        float timeScale;
        float worldMinX, worldMinY, worldWidth, worldHeight;

        SpineInstance() : atlas(nullptr), skeletonData(nullptr), skeleton(nullptr),
            animationStateData(nullptr), animationState(nullptr), timeScale(1.0f),
            worldMinX(0), worldMinY(0), worldWidth(200), worldHeight(200) {}

        void clear() {
            if (animationState) { spAnimationState_dispose(animationState); animationState = nullptr; }
            if (skeleton) { spSkeleton_dispose(skeleton); skeleton = nullptr; }
            if (skeletonData) { spSkeletonData_dispose(skeletonData); skeletonData = nullptr; }
            if (atlas) { spAtlas_dispose(atlas); atlas = nullptr; }
            if (animationStateData) { spAnimationStateData_dispose(animationStateData); animationStateData = nullptr; }


            currentFramePixmap = QPixmap();
            animations.clear();
        }
    };
    SpineInstance currentSpineInstance;
    QElapsedTimer spineDeltaTimer;
    QMap<QString, QString> currentSpineAssetPaths;

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

    QString current_prefix;
    QStringList available_raw_prefixes;
    QMap<QString, CharacterAssetType> prefix_asset_types;
    QMap<QString, QPair<QString, QString>> spine_prefix_files;
    QMap<QString, QString> all_gif_files;
    std::vector<std::string> gif_files_for_current_prefix;
    // Dictionary mapping gif types to their filenames
    QMap<QString, int> gif_type_indices;
    const QStringList known_actions = {"layingalt", "laying", "sit", "walk", "wait", "pick", "move", "lay"};

    // New variables for automatic mode
    QTimer *auto_timer;
    // Behavior variables
    // wait, walk, walk_to_furniture, use_furniture
    QString current_behavior;
    QString move_direction; // Direction of movement
    QString facing_direction; // Direction character is facing (may differ from movement)
    int move_speed;
    QTimer *move_timer;
    bool moving;

    // Furniture variables
    Furniture* current_furniture;
    // Add furniture types dictionary to store available furniture
    QMap<QString, QMap<QString, QVariant>> furniture_types;
    int furniture_target_x;
    // Add furniture state timer for minimum duration
    QTimer *furniture_use_timer;
    // Track whether the character just walked to furniture
    bool walked_to_furniture;

    // Create menu window (but don't show it yet)
    QPointer<MenuWindow> menu_window;
    bool menu_visible;

    QImage crop_image(const QImage &img);
    void discover_animations_and_prefixes();
    void filter_and_load_assets(const QString& prefix);
    void load_current_character_animation();
    void categorize_gifs();
    QString map_action_to_spine_animation_name(const QString& action);

    void load_current_spine_asset();
    void render_spine_skeleton_to_pixmap();
    void update_spine_bounding_box();
    bool computeAffineTransform(const QPointF src[3], const QPointF dst[3], QTransform& outTransform);

    void raise_window_above_furniture();
    QString extract_prefix_and_action(const QString& filename, QString& action_found);
};

class MenuWindow : public QDialog {
    Q_OBJECT

public:
    MenuWindow(TransparentGifViewer *parentViewer = nullptr);
    void update_button_text();
    void update_prefix_list_selection();

signals:
    void toggleAutoModeSignal();
    void addFurnitureSignal(const QString& type);
    void prefixSelectedSignal(const QString& prefix);

private slots:
    void toggle_auto_mode_slot();
    void add_couch();
    void add_table();
    void quit_application();
    void on_prefix_selected();

private:
    QString capitalize(const QString& str);
    TransparentGifViewer *parentViewer;
    QPushButton *auto_mode_btn;
    QListWidget *prefix_list_widget;
};

MenuWindow::MenuWindow(TransparentGifViewer *viewer)
    : QDialog(viewer), parentViewer(viewer) {
    setWindowTitle("ChibiMate");
    setStyleSheet("background-color: rgba(50, 50, 50, 255); color: white;");
    setMinimumSize(320, 400);

    // Layout
    QGridLayout *layout = new QGridLayout(this);

    // Menu title
    QLabel *title = new QLabel("ChibiMate", this);
    title->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title, 0, 0, 1, 2);

    QLabel *prefix_label = new QLabel("Select Character:", this);
    prefix_label->setStyleSheet("color: white; margin-top: 10px;");
    layout->addWidget(prefix_label, 1, 0, 1, 2);

    prefix_list_widget = new QListWidget(this);
    prefix_list_widget->setStyleSheet("background-color: #333; color: white; border: 1px solid #555;");
    layout->addWidget(prefix_list_widget, 2, 0, 1, 2);

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
    connect(prefix_list_widget, &QListWidget::currentItemChanged, this, &MenuWindow::on_prefix_selected);

    // Toggle auto mode button
    auto_mode_btn = new QPushButton("Toggle Auto Mode", this);
    auto_mode_btn->setStyleSheet("color: white; background-color: #444; margin-top: 10px;");
    connect(auto_mode_btn, &QPushButton::clicked, this, &MenuWindow::toggle_auto_mode_slot);
    layout->addWidget(auto_mode_btn, 3, 0, 1, 2);

    // Furniture label
    QLabel* furniture_label = new QLabel("Add Furniture:", this);
    furniture_label->setStyleSheet("color: white; margin-top: 10px;");
    layout->addWidget(furniture_label, 4, 0, 1, 2);

    // Add couch button
    QPushButton *add_couch_btn = new QPushButton("Add Couch", this);
    add_couch_btn->setStyleSheet("color: white; background-color: #444;");
    connect(add_couch_btn, &QPushButton::clicked, this, &MenuWindow::add_couch);
    layout->addWidget(add_couch_btn, 5, 0);

    // Add table button
    QPushButton *add_table_btn = new QPushButton("Add Table", this);
    add_table_btn->setStyleSheet("color: white; background-color: #444;");
    connect(add_table_btn, &QPushButton::clicked, this, &MenuWindow::add_table);
    layout->addWidget(add_table_btn, 5, 1);

    // Close menu button
    QPushButton *close_menu_btn = new QPushButton("Close Menu", this);
    close_menu_btn->setStyleSheet("color: white; background-color: #444; margin-top: 15px;");
    connect(close_menu_btn, &QPushButton::clicked, this, &MenuWindow::close);
    layout->addWidget(close_menu_btn, 6, 0);

    // Quit application button
    QPushButton *quit_app_btn = new QPushButton("Quit", this);
    quit_app_btn->setStyleSheet("color: white; background-color: #700; margin-top: 15px;");
    connect(quit_app_btn, &QPushButton::clicked, this, &MenuWindow::quit_application);
    layout->addWidget(quit_app_btn, 6, 1);

    adjustSize();
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

void MenuWindow::toggle_auto_mode_slot() {
    emit toggleAutoModeSignal();
    update_button_text();
}

void MenuWindow::add_couch() {
    // Get default offsets from furniture_types if they exist
    // Create the furniture with the default offsets from its configuration
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

TransparentGifViewer::TransparentGifViewer()
    : currentAssetType(TYPE_NONE),
    current_gif_index(0), current_frame(0),
    animation_timer(new QTimer(this)), dragging(false),
    pre_drag_auto_mode(false),
    current_prefix(""), auto_mode(false), auto_timer(new QTimer(this)),
    current_behavior("wait"), move_direction("right"), facing_direction("right"),
    move_speed(5), move_timer(new QTimer(this)), moving(false),
    current_furniture(nullptr), furniture_target_x(0),
    furniture_use_timer(new QTimer(this)), walked_to_furniture(false),
    menu_visible(false), pick_gif_index(-1) {
    setWindowTitle("ChibiMate");
    // Ensure the character window is always on top
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background-color: rgba(30, 30, 30, 0);");

    // Main layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    image_label = new QLabel(this);
    image_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(image_label);
    setMouseTracking(true);

    connect(animation_timer, &QTimer::timeout, this, &TransparentGifViewer::next_character_frame_slot);
    connect(auto_timer, &QTimer::timeout, this, &TransparentGifViewer::auto_change_state);
    connect(move_timer, &QTimer::timeout, this, &TransparentGifViewer::move_window);
    connect(furniture_use_timer, &QTimer::timeout, this, &TransparentGifViewer::finish_using_furniture);
    furniture_use_timer->setSingleShot(true);
    connect(this, &TransparentGifViewer::animation_error, this, &TransparentGifViewer::show_error);

    furniture_types.clear();

    QMap<QString, QVariant> couch_props;
    couch_props["filename"] = "couch.png";
    couch_props["use_point_x"] = 100; couch_props["use_point_y"] = 200;
    couch_props["use_type"] = "sit"; couch_props["x_offset"] = 0; couch_props["y_offset"] = -70;
    QString couch_image_path = QCoreApplication::applicationDirPath() + "/" + couch_props["filename"].toString();
    if (QFile::exists(couch_image_path)) {
        furniture_types["couch"] = couch_props;
    } else {
#ifndef QT_NO_DEBUG
        qDebug() << "Couch texture not found (" << couch_image_path << "), couch furniture type will not be available.";
#endif
    }

    QMap<QString, QVariant> table_props;
    table_props["filename"] = "table.png";
    QString table_image_path = QCoreApplication::applicationDirPath() + "/" + table_props["filename"].toString();
    if (QFile::exists(table_image_path)) {
        table_props["use_point_x"] = 50; table_props["use_point_y"] = 150;
        table_props["use_type"] = "sit"; table_props["x_offset"] = 0; table_props["y_offset"] = -50;
        furniture_types["table"] = table_props;
    } else {
#ifndef QT_NO_DEBUG
        qDebug() << "Table texture not found (" << table_image_path << "), table furniture type will not be available.";
#endif
    }

    discover_animations_and_prefixes();

    QString initial_prefix = "";
    if (!available_raw_prefixes.isEmpty()) {
        initial_prefix = available_raw_prefixes.first();
    }

    menu_window = new MenuWindow(this);
    connect(menu_window, &MenuWindow::toggleAutoModeSignal, this, &TransparentGifViewer::toggle_auto_mode);
    connect(menu_window, &MenuWindow::addFurnitureSignal, this, &TransparentGifViewer::handle_add_furniture);
    connect(menu_window, &MenuWindow::prefixSelectedSignal, this, &TransparentGifViewer::set_character_prefix);
    connect(menu_window, &QDialog::finished, this, [this](){ menu_visible = false; });

    // Load the GIFs and categorize them (Done by filter_and_load_assets)
    if (!initial_prefix.isEmpty()) {
        filter_and_load_assets(initial_prefix);
        show();
    } else {
        QLabel *errorLabel = new QLabel("Error: No character assets found.\n"
                                        "Add 'prefixAction.gif' or 'prefix.json/.atlas'\n"
                                        "to the application directory.", this);
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLabel->setStyleSheet("background-color: rgba(50,0,0,200); color: white; padding: 20px; font-size: 14px;");
        layout->addWidget(errorLabel);
        image_label->hide();
        setFixedSize(400, 150);
        setStyleSheet("background-color: rgba(50,0,0,180);");
        show();
    }

    QShortcut *escapeShortcut = new QShortcut(Qt::Key_Escape, this);
    connect(escapeShortcut, &QShortcut::activated, qApp, &QApplication::quit);
}

TransparentGifViewer::~TransparentGifViewer() {
    if (current_furniture) {
        delete current_furniture;
        current_furniture = nullptr;
    }
    currentSpineInstance.clear();
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
    all_gif_files.clear();
    spine_prefix_files.clear();
    prefix_asset_types.clear();
    available_raw_prefixes.clear();
    std::set<QString> unique_prefixes_set;

    QDir appDir(QCoreApplication::applicationDirPath());
    QDirIterator it(appDir.path(), QStringList() << "*.gif" << "*.webp" << "*.json", QDir::Files, QDirIterator::NoIteratorFlags);

    int file_count = 0;
    while (it.hasNext()) {
        it.next();
        file_count++;
        QString filepath = it.filePath();
        QString filename = it.fileName();
        QFileInfo fileInfo(filename);
        QString baseName = fileInfo.completeBaseName();

        if (filename.endsWith(".gif") || filename.endsWith(".webp")) {
            QString action_found;
            QString prefix = extract_prefix_and_action(filename, action_found);
            if (!prefix.isEmpty() && !action_found.isEmpty()) {
                all_gif_files[filename] = prefix;
                if (!prefix_asset_types.contains(prefix) || prefix_asset_types[prefix] == TYPE_GIF) {
                    prefix_asset_types[prefix] = TYPE_GIF;
                    unique_prefixes_set.insert(prefix);
                }
            }
        } else if (filename.endsWith(".json")) {
            QString prefix = baseName;
            QString atlas_filename = prefix + ".atlas";
            QString atlas_filepath = appDir.filePath(atlas_filename);

            if (QFile::exists(atlas_filepath)) {
                if (!prefix.isEmpty()) {
                    spine_prefix_files[prefix] = qMakePair(filepath, atlas_filepath);
                    prefix_asset_types[prefix] = TYPE_SPINE;
                    unique_prefixes_set.insert(prefix);
#ifndef QT_NO_DEBUG
                    qDebug() << "Found Spine character:" << prefix << "with json:" << filename << "and atlas:" << atlas_filename;
#endif
                }
            } else {
#ifndef QT_NO_DEBUG
                qDebug() << "Found Spine json:" << filename << "but missing .atlas file:" << atlas_filename;
#endif
            }
        }
    }
    available_raw_prefixes = QStringList(unique_prefixes_set.begin(), unique_prefixes_set.end());
    available_raw_prefixes.sort();
#ifndef QT_NO_DEBUG
    qDebug() << "Found" << file_count << "relevant files.";
    qDebug() << "Available prefixes:" << available_raw_prefixes;
#endif
}

void TransparentGifViewer::filter_and_load_assets(const QString& prefix_to_load) {
    if (prefix_to_load.isEmpty()) {
        emit animation_error("Cannot load empty prefix.");
        return;
    }
#ifndef QT_NO_DEBUG
    qDebug() << "Loading assets for prefix:" << prefix_to_load;
#endif

    if (prefix_to_load == current_prefix && currentAssetType != TYPE_NONE) {
        return;
    }

    current_prefix = prefix_to_load;
    currentSpineInstance.clear();
    gif_files_for_current_prefix.clear();
    currentAssetType = TYPE_NONE;

    if (!prefix_asset_types.contains(prefix_to_load)) {
        emit animation_error("Unknown prefix or asset type for: " + prefix_to_load);
        image_label->clear(); resize(100,100); return;
    }
    currentAssetType = prefix_asset_types[prefix_to_load];

    if (current_furniture) finish_using_furniture();
    stop_auto_mode();
    if (menu_window) menu_window->update_button_text();
    moving = false;
    walked_to_furniture = false;
    facing_direction = "right";

    if (currentAssetType == TYPE_GIF) {
        for (auto it_gif = all_gif_files.constBegin(); it_gif != all_gif_files.constEnd(); ++it_gif) {
            if (it_gif.value() == prefix_to_load) {
                gif_files_for_current_prefix.push_back(it_gif.key().toStdString());
            }
        }
        if (gif_files_for_current_prefix.empty()) {
            emit animation_error("No GIF animations found for character: " + prefix_to_load);
            image_label->clear(); resize(100,100); currentAssetType = TYPE_NONE; return;
        }
        categorize_gifs();
        current_gif_index = gif_type_indices.value("wait", 0);
    } else if (currentAssetType == TYPE_SPINE) {
        if (!spine_prefix_files.contains(prefix_to_load)) {
            emit animation_error("No Spine files configured for character: " + prefix_to_load);
            image_label->clear(); resize(100,100); currentAssetType = TYPE_NONE; return;
        }
        currentSpineAssetPaths["json"] = spine_prefix_files[prefix_to_load].first;
        currentSpineAssetPaths["atlas"] = spine_prefix_files[prefix_to_load].second;
    } else {
        emit animation_error("No assets found for prefix: " + prefix_to_load);
        image_label->clear(); resize(100,100); return;
    }

    current_behavior = "wait";
    load_current_character_animation();

    if (menu_window && menu_window->isVisible()) {
        menu_window->update_prefix_list_selection();
    }
}

void TransparentGifViewer::categorize_gifs() {
    gif_type_indices.clear();
    gif_type_indices["wait"] = -1; gif_type_indices["walk"] = -1; gif_type_indices["sit"] = -1;
    gif_type_indices["laying"] = -1; gif_type_indices["pick"] = -1;

    pick_gif_index = -1;

    for (size_t i = 0; i < gif_files_for_current_prefix.size(); ++i) {
        QString filename = QString::fromStdString(gif_files_for_current_prefix[i]);
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
            if(categorized) qDebug() << "GIF Categorized:" << action_found << "at index" << i << "File:" << filename;
#endif
        }
    }
    if (gif_type_indices.value("walk", -1) == -1 && gif_type_indices.value("move", -1) != -1) {
        gif_type_indices["walk"] = gif_type_indices["move"];
    }
    if (gif_type_indices.value("laying", -1) == -1) {
        if (gif_type_indices.value("lay", -1) != -1) {
            gif_type_indices["laying"] = gif_type_indices["lay"];
        } else if (gif_type_indices.value("layingalt", -1) != -1) {
            gif_type_indices["laying"] = gif_type_indices["layingalt"];
        }
    }

    int fallback_wait_index = 0;
    if (!gif_files_for_current_prefix.empty() && gif_type_indices.value("wait", -1) == -1) {
        gif_type_indices["wait"] = fallback_wait_index;
    }

    int wait_idx = gif_type_indices.value("wait", -1);
    if (wait_idx != -1) {
        if (gif_type_indices.value("walk", -1) == -1) gif_type_indices["walk"] = wait_idx;
        if (gif_type_indices.value("sit", -1) == -1) gif_type_indices["sit"] = wait_idx;
        if (gif_type_indices.value("laying", -1) == -1) gif_type_indices["laying"] = wait_idx;
        if (pick_gif_index != -1) gif_type_indices["pick"] = pick_gif_index;
        else if (gif_type_indices.value("pick", -1) == -1) gif_type_indices["pick"] = wait_idx;
    }
}


QStringList TransparentGifViewer::get_available_prefixes() const {
    return available_raw_prefixes;
}

QString TransparentGifViewer::get_current_prefix() const {
    return current_prefix;
}

void TransparentGifViewer::set_character_prefix(const QString& prefix) {
    if (!available_raw_prefixes.contains(prefix) && !prefix_asset_types.contains(prefix)) {
        qWarning() << "Attempted to set unknown prefix:" << prefix;
        return;
    }
    filter_and_load_assets(prefix);
}

QImage TransparentGifViewer::crop_image(const QImage &img) {
    if (img.isNull()) return img;
    int width = img.width();
    int height = img.height();
    int crop_top = static_cast<int>(height * 0.3);
    if (crop_top >= height || crop_top < 0) return QImage();
    return img.copy(0, crop_top, width, height - crop_top);
}


void TransparentGifViewer::load_current_character_animation() {
    animation_timer->stop();
    image_label->clear();

    if (currentAssetType == TYPE_GIF) {
        if (gif_files_for_current_prefix.empty()) {
            emit animation_error("No GIFs available for " + current_prefix);
            resize(100,100); return;
        }
        current_gif_index = gif_type_indices.value(current_behavior, gif_type_indices.value("wait", 0));
        if (current_gif_index < 0 || current_gif_index >= static_cast<int>(gif_files_for_current_prefix.size())) {
            current_gif_index = gif_type_indices.value("wait",0);
            if (gif_files_for_current_prefix.empty() || current_gif_index < 0 || current_gif_index >= static_cast<int>(gif_files_for_current_prefix.size())) {
                emit animation_error("Cannot load any GIF index for " + current_prefix + " behavior " + current_behavior);
                resize(100,100); return;
            }
        }

        frames.clear();
        flipped_frames.clear();
        durations.clear();
        current_frame = 0;

        QString current_file_name = QString::fromStdString(gif_files_for_current_prefix[current_gif_index]);
#ifndef QT_NO_DEBUG
        qDebug() << "Loading GIF:" << current_file_name << "for behavior" << current_behavior;
#endif
        QString file_path = QCoreApplication::applicationDirPath() + "/" + current_file_name;
        QMovie movie(file_path);

        if (!movie.isValid()) {
            emit animation_error(QString("Error loading GIF (invalid movie): ") + current_file_name);
            resize(100,100); return;
        }

        int frame_count_from_movie = movie.frameCount();
        if (frame_count_from_movie <= 0) {
            QPixmap pixmap = movie.currentPixmap();
            if (!pixmap.isNull()) {
                QImage current_img = pixmap.toImage();
                current_img = crop_image(current_img);
                if (!current_img.isNull()) {
                    frames.push_back(QPixmap::fromImage(current_img));
                    // Also create flipped version for left movement
                    flipped_frames.push_back(frames[0].transformed(QTransform().scale(-1, 1)));
                    durations.push_back(1000);
                } else { emit animation_error(QString("Error cropping static image: ") + current_file_name); resize(100,100); return; }
            } else { emit animation_error(QString("No frames or invalid image in: ") + current_file_name); resize(100,100); return; }
        } else {
            movie.jumpToFrame(0);
            for (int i = 0; i < frame_count_from_movie; ++i) {
                QImage current_img = movie.currentImage();
                if (current_img.isNull()) {
                    if (i < frame_count_from_movie -1 && !movie.jumpToNextFrame()) break;
                    continue;
                }
                QImage cropped_img = crop_image(current_img);
                if (cropped_img.isNull()) {
                    if (i < frame_count_from_movie -1 && !movie.jumpToNextFrame()) break;
                    continue;
                }
                QPixmap pixmap_frame = QPixmap::fromImage(cropped_img);
                frames.push_back(pixmap_frame);
                // Also create flipped version for left movement
                flipped_frames.push_back(pixmap_frame.transformed(QTransform().scale(-1, 1)));
                durations.push_back(movie.nextFrameDelay());
                if (i < frame_count_from_movie - 1) {
                    if (!movie.jumpToNextFrame()) break;
                }
            }
        }

        if (frames.empty()) {
            emit animation_error(QString("No valid frames extracted from ") + current_file_name);
            resize(100,100); return;
        }

        resize(frames[0].width(), frames[0].height());
        image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);
        image_label->resize(frames[0].size());
        image_label->setGeometry(0, 0, width(), height());
        start_animation();

    } else if (currentAssetType == TYPE_SPINE) {
#ifndef QT_NO_DEBUG
        qDebug() << "Loading Spine asset for prefix:" << current_prefix << "behavior:" << current_behavior;
#endif
        load_current_spine_asset();
    }
}


void TransparentGifViewer::load_current_spine_asset() {
    currentSpineInstance.clear();

    QByteArray atlasPathUtf8 = currentSpineAssetPaths["atlas"].toUtf8();
    const char* atlasPathC = atlasPathUtf8.constData();

    QByteArray jsonPathUtf8 = currentSpineAssetPaths["json"].toUtf8();
    const char* jsonPathC = jsonPathUtf8.constData();

#ifndef QT_NO_DEBUG
    qDebug() << "Attempting to load atlas with C-string path:" << atlasPathC;
    if (atlasPathUtf8.isEmpty()) {
        qWarning() << "Atlas path QString was empty or resulted in empty QByteArray!";
    }
#endif

    currentSpineInstance.atlas = spAtlas_createFromFile(atlasPathC, nullptr);

    if (!currentSpineInstance.atlas) {
        emit animation_error("Error loading Spine atlas: " + currentSpineAssetPaths["atlas"]);
        return;
    }

    spSkeletonJson* jsonParser = spSkeletonJson_create(currentSpineInstance.atlas);
    jsonParser->scale = 1.0f;

#ifndef QT_NO_DEBUG
    qDebug() << "Attempting to read skeleton data file with C-string path:" << jsonPathC;
    if (jsonPathUtf8.isEmpty()) {
        qWarning() << "JSON path QString was empty or resulted in empty QByteArray!";
    }
#endif
    currentSpineInstance.skeletonData = spSkeletonJson_readSkeletonDataFile(jsonParser, jsonPathC);

    if (!currentSpineInstance.skeletonData) {
        QString errorMsg = "Error loading Spine skeleton data: " + currentSpineAssetPaths["json"];
        if (jsonParser->error) errorMsg += QString(" - Spine Error: ") + jsonParser->error;
        emit animation_error(errorMsg);
        spSkeletonJson_dispose(jsonParser);
        if (currentSpineInstance.atlas) {
            spAtlas_dispose(currentSpineInstance.atlas);
            currentSpineInstance.atlas = nullptr;
        }
        return;
    }
    spSkeletonJson_dispose(jsonParser);

    currentSpineInstance.skeleton = spSkeleton_create(currentSpineInstance.skeletonData);
    spSkeleton_setToSetupPose(currentSpineInstance.skeleton);
    spSkeleton_updateWorldTransform(currentSpineInstance.skeleton);

    currentSpineInstance.animationStateData = spAnimationStateData_create(currentSpineInstance.skeletonData);
    currentSpineInstance.animationStateData->defaultMix = 0.2f;
    currentSpineInstance.animationState = spAnimationState_create(currentSpineInstance.animationStateData);

    currentSpineInstance.animations.clear();
    for (int i = 0; i < currentSpineInstance.skeletonData->animationsCount; ++i) {
        spAnimation* anim = currentSpineInstance.skeletonData->animations[i];
        currentSpineInstance.animations[QString(anim->name)] = anim;
    }

    QString initial_anim_name = map_action_to_spine_animation_name(current_behavior);
    if (!initial_anim_name.isEmpty() && currentSpineInstance.animations.contains(initial_anim_name)) {
        spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, initial_anim_name.toUtf8().constData(), true);
    } else if (currentSpineInstance.skeletonData->animationsCount > 0) {
        initial_anim_name = QString(currentSpineInstance.skeletonData->animations[0]->name);
        spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, initial_anim_name.toUtf8().constData(), true);
#ifndef QT_NO_DEBUG
        qDebug() << "Spine: Initial animation" << initial_anim_name << "set as fallback for behavior" << current_behavior;
#endif
    } else {
        emit animation_error("Spine: No animations found for " + current_prefix);
    }

    update_spine_bounding_box();

    spAnimationState_update(currentSpineInstance.animationState, 0.0f);
    spAnimationState_apply(currentSpineInstance.animationState, currentSpineInstance.skeleton);
    spSkeleton_updateWorldTransform(currentSpineInstance.skeleton);

    render_spine_skeleton_to_pixmap();
    if (!currentSpineInstance.currentFramePixmap.isNull()) {
        image_label->setPixmap(currentSpineInstance.currentFramePixmap);
    }

    spineDeltaTimer.start();
    start_animation();
}


void TransparentGifViewer::update_spine_bounding_box() {
    const float fixedRenderWidth = 340.0f;
    const float fixedRenderHeight = 340.0f;
    const float cropPercentage = 0.3f;
    const float croppedRenderHeight = fixedRenderHeight * (1.0f - cropPercentage);

    const QColor spineCharacterBackgroundColor(30, 30, 30, 30);

    if (!currentSpineInstance.skeleton) {
#ifndef QT_NO_DEBUG
        qDebug() << "Spine: Cannot update bounds, skeleton is null.";
#endif
        currentSpineInstance.worldWidth = fixedRenderWidth;
        currentSpineInstance.worldHeight = fixedRenderHeight;
        currentSpineInstance.worldMinX = -fixedRenderWidth / 2.0f;
        currentSpineInstance.worldMinY = -fixedRenderHeight / 2.0f;

        int pixmapWidth = qMax(1, (int)ceil(fixedRenderWidth));
        int pixmapHeight = qMax(1, (int)ceil(croppedRenderHeight));

        if (currentSpineInstance.currentFramePixmap.width() != pixmapWidth ||
            currentSpineInstance.currentFramePixmap.height() != pixmapHeight) {
            currentSpineInstance.currentFramePixmap = QPixmap(pixmapWidth, pixmapHeight);
        }
        if (!currentSpineInstance.currentFramePixmap.isNull()) {
            currentSpineInstance.currentFramePixmap.fill(spineCharacterBackgroundColor);
        }
        resize(pixmapWidth, pixmapHeight);
        if(image_label) {
            image_label->resize(pixmapWidth, pixmapHeight);
            image_label->setGeometry(0, 0, pixmapWidth, pixmapHeight);
        }
#ifndef QT_NO_DEBUG
        qDebug() << "Spine pixmap (no skeleton or simplified) resized to:" << pixmapWidth << "x" << pixmapHeight;
#endif
        return;
    }

    spSkeletonBounds* bounds = spSkeletonBounds_create();
    spSkeleton_setToSetupPose(currentSpineInstance.skeleton);
    spSkeleton_updateWorldTransform(currentSpineInstance.skeleton);
    spSkeletonBounds_update(bounds, currentSpineInstance.skeleton, true);

    float skelOriginalWidth = bounds->maxX - bounds->minX;
    float skelOriginalHeight = bounds->maxY - bounds->minY;

    bool boundsAreMeaningful = (bounds->minX != (float)INT_MAX && bounds->minY != (float)INT_MAX &&
                                skelOriginalWidth >= 1.0f && skelOriginalHeight >= 1.0f);

    if (!boundsAreMeaningful) {
        if (currentSpineInstance.skeletonData && currentSpineInstance.skeletonData->width > 0 && currentSpineInstance.skeletonData->height > 0) {
#ifndef QT_NO_DEBUG
            qDebug() << "Spine: Meaningful bounds not found. Falling back to skeletonData width/height:"
                     << currentSpineInstance.skeletonData->width << "x" << currentSpineInstance.skeletonData->height;
#endif
            skelOriginalWidth = currentSpineInstance.skeletonData->width;
            skelOriginalHeight = currentSpineInstance.skeletonData->height;
            currentSpineInstance.worldMinX = (currentSpineInstance.skeleton->x - skelOriginalWidth / 2.0f) - (fixedRenderWidth - skelOriginalWidth) / 2.0f;
            currentSpineInstance.worldMinY = (currentSpineInstance.skeleton->y - skelOriginalHeight / 2.0f) - (fixedRenderHeight - skelOriginalHeight) / 2.0f;
        } else {
#ifndef QT_NO_DEBUG
            qDebug() << "Spine: No meaningful bounds or skeletonData width/height. Defaulting to conceptual 200x200 centered in 340x340.";
#endif
            float defaultFallbackWidth = 200.0f;
            float defaultFallbackHeight = 200.0f;
            currentSpineInstance.worldMinX = currentSpineInstance.skeleton->x - defaultFallbackWidth / 2.0f - (fixedRenderWidth - defaultFallbackWidth) / 2.0f;
            currentSpineInstance.worldMinY = currentSpineInstance.skeleton->y - defaultFallbackHeight / 2.0f - (fixedRenderHeight - defaultFallbackHeight) / 2.0f;
        }
    } else {
        currentSpineInstance.worldMinX = bounds->minX - (fixedRenderWidth - skelOriginalWidth) / 2.0f;
        currentSpineInstance.worldMinY = bounds->minY - (fixedRenderHeight - skelOriginalHeight) / 2.0f;
#ifndef QT_NO_DEBUG
        qDebug() << "Spine: Bounds calculated from attachments:" << skelOriginalWidth << "x" << skelOriginalHeight
                 << "at skelMinX:" << bounds->minX << "skelMinY:" << bounds->minY;
#endif
    }
    spSkeletonBounds_dispose(bounds);

    currentSpineInstance.worldWidth = fixedRenderWidth;
    currentSpineInstance.worldHeight = fixedRenderHeight;

    int pixmapWidth = qMax(1, (int)ceil(fixedRenderWidth));
    int pixmapHeight = qMax(1, (int)ceil(croppedRenderHeight));

    if (currentSpineInstance.currentFramePixmap.width() != pixmapWidth ||
        currentSpineInstance.currentFramePixmap.height() != pixmapHeight) {
        currentSpineInstance.currentFramePixmap = QPixmap(pixmapWidth, pixmapHeight);
    }

    if (!currentSpineInstance.currentFramePixmap.isNull()) {
        currentSpineInstance.currentFramePixmap.fill(Qt::transparent);
    }

    resize(pixmapWidth, pixmapHeight);
    if(image_label) {
        image_label->resize(pixmapWidth, pixmapHeight);
        image_label->setGeometry(0, 0, pixmapWidth, pixmapHeight);
    }
#ifndef QT_NO_DEBUG
    qDebug() << "Spine pixmap final size (cropped):" << pixmapWidth << "x" << pixmapHeight
             << ". Widget size:" << width() << "x" << height();
    qDebug() << "Spine worldMinX:" << currentSpineInstance.worldMinX << "worldMinY:" << currentSpineInstance.worldMinY;
    qDebug() << "Spine conceptual worldWidth:" << currentSpineInstance.worldWidth << "conceptual worldHeight:" << currentSpineInstance.worldHeight;
#endif
}

QString TransparentGifViewer::map_action_to_spine_animation_name(const QString& action) {
    if (!currentSpineInstance.skeleton) return QString();

    if (action.toLower() == "wait") {
        if (currentSpineInstance.animations.contains("idle")) return "idle";
        if (currentSpineInstance.animations.contains("wait")) return "wait";
    } else if (action.toLower() == "walk") {
        if (currentSpineInstance.animations.contains("walk")) return "walk";
        if (currentSpineInstance.animations.contains("run")) return "run";
        if (currentSpineInstance.animations.contains("move")) return "move";
    } else if (action.toLower() == "sit") {
        if (currentSpineInstance.animations.contains("sit")) return "sit";
    } else if (action.toLower() == "laying") {
        if (currentSpineInstance.animations.contains("lay")) return "lay";
        if (currentSpineInstance.animations.contains("laying")) return "laying";
    } else if (action.toLower() == "pick") {
        if (currentSpineInstance.animations.contains("pick")) return "pick";
        if (currentSpineInstance.animations.contains("pickup")) return "pickup";
    }
    if (currentSpineInstance.animations.contains(action)) return action;

#ifndef QT_NO_DEBUG
    qDebug() << "Spine: Could not map action" << action << "to a known animation name.";
#endif
    if (currentSpineInstance.skeletonData && currentSpineInstance.skeletonData->animationsCount > 0) {
        return QString(currentSpineInstance.skeletonData->animations[0]->name);
    }
    return QString();
}

bool TransparentGifViewer::computeAffineTransform(const QPointF src[3], const QPointF dst[3], QTransform& transform) {
    qreal u1 = src[0].x(), v1 = src[0].y();
    qreal u2 = src[1].x(), v2 = src[1].y();
    qreal u3 = src[2].x(), v3 = src[2].y();

    qreal p1x = dst[0].x(), p1y = dst[0].y();
    qreal p2x = dst[1].x(), p2y = dst[1].y();
    qreal p3x = dst[2].x(), p3y = dst[2].y();

    QTransform srcToDst;

    qreal det = u1 * (v2 - v3) + u2 * (v3 - v1) + u3 * (v1 - v2);

    if (qAbs(det) < 1e-9) {
#ifndef QT_NO_DEBUG
        qDebug() << "computeAffineTransform: Degenerate source triangle, det:" << det;
#endif
        return false;
    }

    qreal m11 = (p1x * (v2 - v3) + p2x * (v3 - v1) + p3x * (v1 - v2)) / det;
    qreal m12 = (p1y * (v2 - v3) + p2y * (v3 - v1) + p3y * (v1 - v2)) / det;

    qreal m21 = (p1x * (u3 - u2) + p2x * (u1 - u3) + p3x * (u2 - u1)) / det;
    qreal m22 = (p1y * (u3 - u2) + p2y * (u1 - u3) + p3y * (u2 - u1)) / det;

    qreal dx = (p1x * (u2 * v3 - u3 * v2) + p2x * (u3 * v1 - u1 * v3) + p3x * (u1 * v2 - u2 * v1)) / det;
    qreal dy = (p1y * (u2 * v3 - u3 * v2) + p2y * (u3 * v1 - u1 * v3) + p3y * (u1 * v2 - u2 * v1)) / det;

    transform.setMatrix(m11, m12, 0,
                        m21, m22, 0,
                        dx,  dy,  1);

    return true;
}

void TransparentGifViewer::render_spine_skeleton_to_pixmap() {
    if (!currentSpineInstance.skeleton || currentSpineInstance.currentFramePixmap.isNull()) {
        return;
    }

    const QColor spineCharacterBackgroundColor(30, 30, 30, 30);

    currentSpineInstance.currentFramePixmap.fill(spineCharacterBackgroundColor);

    QPainter painter(&currentSpineInstance.currentFramePixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    float cropAmount = currentSpineInstance.worldHeight * 0.3f;

    painter.translate(-currentSpineInstance.worldMinX,
                      currentSpineInstance.worldMinY + currentSpineInstance.worldHeight);
    painter.scale(1, -1);

    float worldVerticesBuffer[1000];

    for (int i = 0; i < currentSpineInstance.skeleton->slotsCount; ++i) {
        spSlot* slot = currentSpineInstance.skeleton->drawOrder[i];
        spAttachment* attachment = slot->attachment;
        if (!attachment) continue;

        QImage* textureQImage = nullptr;
        float skeletonR = currentSpineInstance.skeleton->r;
        float skeletonG = currentSpineInstance.skeleton->g;
        float skeletonB = currentSpineInstance.skeleton->b;
        float skeletonA = currentSpineInstance.skeleton->a;

        float slotR = slot->r;
        float slotG = slot->g;
        float slotB = slot->b;
        float slotA = slot->a;

        float tintR = skeletonR * slotR;
        float tintG = skeletonG * slotG;
        float tintB = skeletonB * slotB;
        float tintA = skeletonA * slotA;

        if (tintA == 0.0f) continue;

        if (attachment->type == SP_ATTACHMENT_REGION) {
            spRegionAttachment* region = (spRegionAttachment*)attachment;
            if (region->rendererObject)
                textureQImage = static_cast<QImage*>(((spAtlasRegion*)region->rendererObject)->page->rendererObject);
        } else if (attachment->type == SP_ATTACHMENT_MESH) {
            spMeshAttachment* mesh = (spMeshAttachment*)attachment;
            if (mesh->rendererObject)
                textureQImage = static_cast<QImage*>(((spAtlasRegion*)mesh->rendererObject)->page->rendererObject);
        } else if (attachment->type == SP_ATTACHMENT_SKINNED_MESH) {
            spSkinnedMeshAttachment* skinnedMesh = (spSkinnedMeshAttachment*)attachment;
            if (skinnedMesh->rendererObject)
                textureQImage = static_cast<QImage*>(((spAtlasRegion*)skinnedMesh->rendererObject)->page->rendererObject);
        }
        if (!textureQImage || textureQImage->isNull()) continue;

        if (attachment->type == SP_ATTACHMENT_REGION) {
            spRegionAttachment* regionAtt = (spRegionAttachment*)attachment;
            spAtlasRegion* atlasRegion = (spAtlasRegion*)regionAtt->rendererObject;

            if (!atlasRegion || !textureQImage || textureQImage->isNull()) {
                continue;
            }

            spRegionAttachment_computeWorldVertices(regionAtt, slot->bone, worldVerticesBuffer);
            QPolygonF screenQuad;
            screenQuad << QPointF(worldVerticesBuffer[0], worldVerticesBuffer[1])
                       << QPointF(worldVerticesBuffer[2], worldVerticesBuffer[3])
                       << QPointF(worldVerticesBuffer[4], worldVerticesBuffer[5])
                       << QPointF(worldVerticesBuffer[6], worldVerticesBuffer[7]);

            float copyWidth = atlasRegion->width;
            float copyHeight = atlasRegion->height;

            if (atlasRegion->rotate) {
                copyWidth = atlasRegion->height;
                copyHeight = atlasRegion->width;
            }

            QRectF sourceRectToCopy(atlasRegion->x, atlasRegion->y, copyWidth, copyHeight);
            QPixmap regionOnlyPixmap = QPixmap::fromImage(*textureQImage).copy(sourceRectToCopy.toRect());

            if (regionOnlyPixmap.isNull()) {
                continue;
            }

            if (atlasRegion->rotate) {
                QTransform rotator;
                rotator.rotate(90.0);
                regionOnlyPixmap = regionOnlyPixmap.transformed(rotator, Qt::SmoothTransformation);
            }

            QPolygonF sourcePixmapLocalCorners;
            sourcePixmapLocalCorners << QPointF(0, regionOnlyPixmap.height())
                                     << QPointF(0, 0)
                                     << QPointF(regionOnlyPixmap.width(), 0)
                                     << QPointF(regionOnlyPixmap.width(), regionOnlyPixmap.height());

            QTransform mappingTransform;
            if (QTransform::quadToQuad(sourcePixmapLocalCorners, screenQuad, mappingTransform)) {
                painter.save();
                painter.setTransform(mappingTransform, true);
                painter.setOpacity(tintA);
                painter.drawPixmap(QPointF(0,0), regionOnlyPixmap);
                painter.restore();
            } else {
            }
        } else if (attachment->type == SP_ATTACHMENT_MESH || attachment->type == SP_ATTACHMENT_SKINNED_MESH) {
            float* attachmentWorldVertices = nullptr;
            float* uvs = nullptr;
            int numFloatVertices = 0;
            int numFloatUVs = 0;
            int* triangles = nullptr;
            int trianglesCount = 0;

            if (attachment->type == SP_ATTACHMENT_MESH) {
                spMeshAttachment* mesh = (spMeshAttachment*)attachment;
                numFloatVertices = mesh->verticesCount;
                if (numFloatVertices == 0 || numFloatVertices > (int)(sizeof(worldVerticesBuffer)/sizeof(float))) continue;
                spMeshAttachment_computeWorldVertices(mesh, slot, worldVerticesBuffer);
                attachmentWorldVertices = worldVerticesBuffer;
                uvs = mesh->uvs;
                numFloatUVs = mesh->verticesCount;
                triangles = mesh->triangles;
                trianglesCount = mesh->trianglesCount;
            } else {
                spSkinnedMeshAttachment* skinnedMesh = (spSkinnedMeshAttachment*)attachment;
                numFloatUVs = skinnedMesh->uvsCount;
                if (numFloatUVs == 0 || numFloatUVs > (int)(sizeof(worldVerticesBuffer)/sizeof(float))) continue;
                spSkinnedMeshAttachment_computeWorldVertices(skinnedMesh, slot, worldVerticesBuffer);
                attachmentWorldVertices = worldVerticesBuffer;
                uvs = skinnedMesh->uvs;
                numFloatVertices = numFloatUVs;
                triangles = skinnedMesh->triangles;
                trianglesCount = skinnedMesh->trianglesCount;
            }


            if (!attachmentWorldVertices || !uvs || !textureQImage || textureQImage->isNull()) {
#ifndef QT_NO_DEBUG
                qDebug() << "DEBUG_RENDER: Missing data for MESH/SKINNED_MESH texturing:" << QString(attachment->name);
#endif
                continue;
            }

            painter.setOpacity(tintA);
            QPixmap textureAtlasPixmap = QPixmap::fromImage(*textureQImage);
            if(textureAtlasPixmap.isNull()) continue;

            for (int t = 0; t < trianglesCount; t += 3) {
                QPainterPath path;
                int idx1_v = triangles[t] * 2;
                int idx2_v = triangles[t+1] * 2;
                int idx3_v = triangles[t+2] * 2;

                if (idx1_v + 1 >= numFloatVertices || idx2_v + 1 >= numFloatVertices || idx3_v + 1 >= numFloatVertices ||
                    idx1_v < 0 || idx2_v < 0 || idx3_v < 0 ||
                    idx1_v + 1 >= numFloatUVs || idx2_v + 1 >= numFloatUVs || idx3_v + 1 >= numFloatUVs ) {
#ifndef QT_NO_DEBUG
                    qDebug() << "DEBUG_RENDER: Mesh triangle or UV index out of bounds for" << QString(attachment->name);
#endif
                    continue;
                }

                QPointF p1(attachmentWorldVertices[idx1_v], attachmentWorldVertices[idx1_v + 1]);
                QPointF p2(attachmentWorldVertices[idx2_v], attachmentWorldVertices[idx2_v + 1]);
                QPointF p3(attachmentWorldVertices[idx3_v], attachmentWorldVertices[idx3_v + 1]);
                path.moveTo(p1); path.lineTo(p2); path.lineTo(p3); path.closeSubpath();

                QPointF uv1_tex(uvs[idx1_v] * textureAtlasPixmap.width(), uvs[idx1_v+1] * textureAtlasPixmap.height());
                QPointF uv2_tex(uvs[idx2_v] * textureAtlasPixmap.width(), uvs[idx2_v+1] * textureAtlasPixmap.height());
                QPointF uv3_tex(uvs[idx3_v] * textureAtlasPixmap.width(), uvs[idx3_v+1] * textureAtlasPixmap.height());

                painter.save();
                painter.setClipPath(path, Qt::ReplaceClip);

                QPointF srcPoints[3] = {uv1_tex, uv2_tex, uv3_tex};
                QPointF dstPoints[3] = {p1, p2, p3};
                QTransform affineTexTransform;

                if (computeAffineTransform(srcPoints, dstPoints, affineTexTransform)) {
                    painter.setTransform(affineTexTransform, true);
                    painter.drawPixmap(0,0, textureAtlasPixmap);
                } else {
                    QColor triColorFallback(0, 255, 0, static_cast<int>(tintA * 255 * 0.7));
                    painter.fillPath(path, QBrush(triColorFallback));
#ifndef QT_NO_DEBUG
                    qDebug() << "  DEBUG_RENDER: computeAffineTransform FAILED for MESH triangle:" << QString(attachment->name);
                    qDebug() << "    Src (UV pixels):" << srcPoints[0] << srcPoints[1] << srcPoints[2];
                    qDebug() << "    Dst (Screen):" << dstPoints[0] << dstPoints[1] << dstPoints[2];
#endif
                }
                painter.restore();
            }
        }
    }
    painter.end();

    if (facing_direction == "left") {
        if (!currentSpineInstance.currentFramePixmap.isNull()) {
            currentSpineInstance.currentFramePixmap = currentSpineInstance.currentFramePixmap.transformed(QTransform().scale(-1, 1));
        }
    }
}

void TransparentGifViewer::show_error(const QString &message) {
    qWarning() << "ERROR:" << message;
}

void TransparentGifViewer::start_animation() {
    if (currentAssetType == TYPE_GIF) {
        if (!frames.empty() && !durations.empty() && current_frame < durations.size() && current_frame >= 0) {
            int duration = durations[current_frame] > 0 ? durations[current_frame] : 100;
            animation_timer->start(duration);
        } else if (!frames.empty()) {
            image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);
            animation_timer->stop();
        }
    } else if (currentAssetType == TYPE_SPINE) {
        if (currentSpineInstance.skeleton) {
            spineDeltaTimer.start();
            animation_timer->start(16);
        } else {
            animation_timer->stop();
        }
    }
}

void TransparentGifViewer::next_character_frame_slot() {
    if (currentAssetType == TYPE_GIF) {
        if (frames.empty()) { animation_timer->stop(); return; }
        if (frames.size() == 1) {
            if (animation_timer->isActive()) animation_timer->stop();
            if (!frames.empty()) image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);
            return;
        }

        current_frame = (current_frame + 1) % frames.size();
        // If not moving, update the frame based on the facing direction
        if (current_frame < frames.size() && current_frame < durations.size()) {
            image_label->setPixmap(facing_direction == "right" ? frames[current_frame] : flipped_frames[current_frame]);
            int duration = durations[current_frame] > 0 ? durations[current_frame] : 100;
            animation_timer->start(duration);
        } else if (!frames.empty()) {
            current_frame = 0;
            image_label->setPixmap(facing_direction == "right" ? frames[0] : flipped_frames[0]);
            animation_timer->start(durations.empty() ? 100 : durations[0]);
        } else {
            animation_timer->stop();
        }
    } else if (currentAssetType == TYPE_SPINE) {
        if (!currentSpineInstance.skeleton || !currentSpineInstance.animationState) {
            animation_timer->stop();
            return;
        }

        float delta = spineDeltaTimer.elapsed() / 1000.0f;
        spineDeltaTimer.restart();

        spAnimationState_update(currentSpineInstance.animationState, delta * currentSpineInstance.timeScale);
        spAnimationState_apply(currentSpineInstance.animationState, currentSpineInstance.skeleton);
        spSkeleton_updateWorldTransform(currentSpineInstance.skeleton);

        render_spine_skeleton_to_pixmap();
        if (!currentSpineInstance.currentFramePixmap.isNull()) {
            image_label->setPixmap(currentSpineInstance.currentFramePixmap);
        }
    }
}


void TransparentGifViewer::next_gif() {
    if (currentAssetType == TYPE_GIF) {
        if (gif_files_for_current_prefix.empty()) return;
        current_gif_index = (current_gif_index + 1) % gif_files_for_current_prefix.size();
        load_current_character_animation();
    } else if (currentAssetType == TYPE_SPINE) {
        if (!currentSpineInstance.skeleton || currentSpineInstance.skeletonData->animationsCount == 0) return;
        spTrackEntry* currentEntry = spAnimationState_getCurrent(currentSpineInstance.animationState, 0);
        const char* currentAnimNameC = currentEntry ? currentEntry->animation->name : nullptr;
        int currentAnimIdx = -1;
        if (currentAnimNameC) {
            for (int i = 0; i < currentSpineInstance.skeletonData->animationsCount; ++i) {
                if (strcmp(currentSpineInstance.skeletonData->animations[i]->name, currentAnimNameC) == 0) {
                    currentAnimIdx = i;
                    break;
                }
            }
        }
        currentAnimIdx = (currentAnimIdx + 1) % currentSpineInstance.skeletonData->animationsCount;
        spAnimation* nextAnim = currentSpineInstance.skeletonData->animations[currentAnimIdx];
#ifndef QT_NO_DEBUG
        qDebug() << "Spine: Manually cycling to animation:" << nextAnim->name;
#endif
        spAnimationState_setAnimation(currentSpineInstance.animationState, 0, nextAnim, true);
    }
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
    // Explicitly ensure movement is stopped
    moving = false;
    // Reset behavior to wait
    current_behavior = "wait";
    // Keep the current facing direction (don't reset it)

    // Set to wait animation
    if (currentAssetType == TYPE_GIF) {
        current_gif_index = gif_type_indices.value("wait", 0);
        load_current_character_animation();
    } else if (currentAssetType == TYPE_SPINE) {
        QString wait_anim_name = map_action_to_spine_animation_name("wait");
        if (!wait_anim_name.isEmpty() && currentSpineInstance.animationState) {
            spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, wait_anim_name.toUtf8().constData(), true);
        } else if (currentSpineInstance.skeletonData && currentSpineInstance.skeletonData->animationsCount > 0) {
            spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, currentSpineInstance.skeletonData->animations[0]->name, true);
        }
    }
}


void TransparentGifViewer::auto_change_state() {
    if (!auto_mode) return;
    // Stop current movement if any
    move_timer->stop();
    moving = false;

    QStringList possible_behaviors;
    if (current_behavior == "use_furniture" || current_behavior == "sit" || current_behavior == "laying") {
        possible_behaviors << "wait" << "walk";
        if (current_furniture) {
            finish_using_furniture();
        }
    } else {
        possible_behaviors << "wait" << "walk";
        // Add walk_to_furniture option if no furniture is present
        if (!current_furniture) {
            bool has_couch = furniture_types.contains("couch");
            bool has_table = furniture_types.contains("table");
            // Add use_furniture option only if furniture is visible AND we just walked to it
            if (has_couch || has_table) {
                possible_behaviors << "walk_to_furniture";
            }
        }
    }

    if (possible_behaviors.size() > 1) {
        if (current_behavior == "wait") possible_behaviors.removeAll("wait");
        else if (current_behavior == "walk" || current_behavior == "walk_to_furniture") possible_behaviors.removeAll("walk");
    }
    if (possible_behaviors.isEmpty()) possible_behaviors << "wait";

    QString next_logical_behavior = possible_behaviors.at(QRandomGenerator::global()->bounded(possible_behaviors.size()));
#ifndef QT_NO_DEBUG
    qDebug() << "Auto mode: Current behavior:" << current_behavior << "-> Next random behavior:" << next_logical_behavior;
#endif


    if (next_logical_behavior == "wait") {
        current_behavior = "wait";
        if (currentAssetType == TYPE_GIF) {
            current_gif_index = gif_type_indices.value("wait", 0);
            load_current_character_animation();
        } else if (currentAssetType == TYPE_SPINE) {
            QString anim_name = map_action_to_spine_animation_name("wait");
            if (!anim_name.isEmpty()) spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, anim_name.toUtf8().constData(), true);
        }
    } else if (next_logical_behavior == "walk") {
        start_moving();
    } else if (next_logical_behavior == "walk_to_furniture") {
        // Randomly select furniture type for auto mode
        QStringList available_types = furniture_types.keys();
        if (!available_types.isEmpty()) {
            // Get default offsets from furniture configuration
            create_furniture(available_types.at(QRandomGenerator::global()->bounded(available_types.size())));
        } else {
            current_behavior = "wait";
            if (currentAssetType == TYPE_GIF) {  }
            else if (currentAssetType == TYPE_SPINE) {  }
        }
    } else if (next_logical_behavior == "use_furniture") {
        use_furniture();
    }

    // Choose a random duration between 10-20 seconds
    int duration_min = 10000;
    int duration_max = 20000;
    // In auto mode, when the state changes away from using furniture, clean up
    if (current_behavior == "sit" || current_behavior == "laying" || current_behavior == "use_furniture") {
        auto_timer->stop();
        if (!furniture_use_timer->isActive()) {
            furniture_use_timer->start(QRandomGenerator::global()->bounded(duration_min, duration_max + 1));
        }
    } else {
        auto_timer->start(QRandomGenerator::global()->bounded(duration_min, duration_max + 1));
    }
}


void TransparentGifViewer::start_moving() {
    moving = true;
    current_behavior = "walk";

    // Use the walking animation
    if (currentAssetType == TYPE_GIF) {
        current_gif_index = gif_type_indices.value("walk", gif_type_indices.value("wait",0));
        load_current_character_animation();
    } else if (currentAssetType == TYPE_SPINE) {
        QString anim_name = map_action_to_spine_animation_name("walk");
        if (!anim_name.isEmpty()) spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, anim_name.toUtf8().constData(), true);
    }

    // Randomly choose direction and update facing direction to match
    move_direction = (QRandomGenerator::global()->bounded(2) == 0) ? "left" : "right";
    facing_direction = move_direction;
    // Start the movement timer
    move_timer->start(50); // Update position every 50ms
}


void TransparentGifViewer::start_walk_to_furniture() {
    if (!current_furniture) return;
    moving = true;
    // Reset flag
    walked_to_furniture = false;
    current_behavior = "walk_to_furniture";

    // Use the walking animation
    if (currentAssetType == TYPE_GIF) {
        current_gif_index = gif_type_indices.value("walk", gif_type_indices.value("wait",0));
        load_current_character_animation();
    } else if (currentAssetType == TYPE_SPINE) {
        QString anim_name = map_action_to_spine_animation_name("walk");
        if (!anim_name.isEmpty()) spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, anim_name.toUtf8().constData(), true);
    }

    // Determine initial direction based on furniture position
    int character_center_x = x() + width() / 2;
    // Set the initial direction and lock it for the duration of the walk
    // Also update the facing direction to match
    furniture_target_x = current_furniture->get_center_x();
    move_direction = (furniture_target_x > character_center_x) ? "right" : "left";
    facing_direction = move_direction;
#ifndef QT_NO_DEBUG
    qDebug() << "Starting walk to furniture in" << move_direction << "direction. Target X:" << furniture_target_x;
#endif
    // Start the movement timer
    move_timer->start(50); // Update position every 50ms
}


void TransparentGifViewer::move_window() {
    if (!moving) return;
    QPoint current_pos = pos();
    int current_x = current_pos.x();
    int next_x = current_x;

    // Check if window is about to leave the screen and reverse direction if needed (for "walk" behavior)
    if (current_behavior == "walk") {
        QScreen *screen = QGuiApplication::screenAt(current_pos);
        if (!screen) screen = QGuiApplication::primaryScreen();
        QRect screen_rect = screen->availableGeometry();

        if (move_direction == "right" && (next_x + width()) > screen_rect.right()) {
            next_x = screen_rect.right() - width();
            // About to hit right edge, change direction
            move_direction = "left";
            facing_direction = "left"; // Update facing direction too
        } else if (move_direction == "left" && next_x < screen_rect.left()) {
            next_x = screen_rect.left();
            // About to hit left edge, change direction
            move_direction = "right";
            facing_direction = "right"; // Update facing direction too
        }
    }

    // Handle different movement behaviors
    if (current_behavior == "walk_to_furniture" && current_furniture) {
        // Calculate center point of character
        int character_center_x = current_pos.x() + width() / 2 + (move_direction == "right" ? move_speed : -move_speed);
        // Check if we've reached the target (furniture center)
        if (std::abs(character_center_x - furniture_target_x) < (width() / 4.0) ) {
            // Reached furniture, start using it
#ifndef QT_NO_DEBUG
            qDebug() << "Reached furniture, switching to use_furniture";
#endif \
    // Set the flag indicating we just walked to furniture
            walked_to_furniture = true;
            use_furniture();
            return; // Important: return after calling use_furniture
        }
        // No direction changes in walk_to_furniture mode - keep the initial direction
        // This prevents the back-and-forth flipping issue
    }

    // Move in the current direction and update the character frame
    if (move_direction == "right") {
        next_x = current_x + move_speed;
        facing_direction = "right"; // Ensure facing direction is updated
    } else {
        next_x = current_x - move_speed;
        facing_direction = "left"; // Ensure facing direction is updated
    }
    move(next_x, current_pos.y());

    // Ensure character stays above furniture during movement
    if (current_furniture && current_furniture->visible) {
        raise_window_above_furniture();
    }
}


void TransparentGifViewer::create_furniture(const QString& furniture_type_name) {
    if (!furniture_types.contains(furniture_type_name)) {
#ifndef QT_NO_DEBUG
        qDebug() << "Attempted to create unknown furniture type:" << furniture_type_name;
#endif
        return;
    }
    if (current_furniture) {
        delete current_furniture;
        current_furniture = nullptr;
    }

    QMap<QString, QVariant> props = furniture_types[furniture_type_name];
    // Use specified offsets or defaults from furniture_types
    // Create the furniture object with the offsets
    current_furniture = new Furniture(
        props["filename"].toString(),
        QPoint(props["use_point_x"].toInt(), props["use_point_y"].toInt()),
        props["use_type"].toString(),
        props["x_offset"].toInt(),
        props["y_offset"].toInt()
        );

    if (current_furniture->pixmap.isNull()) {
        delete current_furniture;
        current_furniture = nullptr;
        emit animation_error("Failed to load furniture image: " + props["filename"].toString());
        return;
    }

    QScreen *screen = QGuiApplication::screenAt(pos());
    if (!screen) screen = QGuiApplication::primaryScreen();
    QRect screen_rect = screen->availableGeometry();

    // Get furniture dimensions
    int furniture_width = current_furniture->width;
    int furniture_height = current_furniture->height;
    int margin = 20;

    // Calculate the available screen space to ensure furniture doesn't clip out
    int min_x = screen_rect.left() + margin;
    int max_x = screen_rect.right() - furniture_width - margin;
    // Generate a random position along the x-axis within the valid range
    int base_furniture_x = (max_x >= min_x) ? QRandomGenerator::global()->bounded(min_x, max_x + 1) : min_x;
    // Apply x_offset to the randomized position

    // Calculate the y-position to align bottoms
    int char_bottom = y() + height();
    // Align furniture bottom with character bottom, then apply y_offset
    int base_furniture_y = char_bottom - furniture_height + props["y_offset"].toInt();

    // Ensure the furniture stays within screen bounds after offsets
    int final_x = std::max(min_x, std::min(base_furniture_x, max_x));
    int final_y = std::max(screen_rect.top() + margin, std::min(base_furniture_y, screen_rect.bottom() - furniture_height - margin));

    // Set the furniture position and show it
    current_furniture->set_position(final_x, final_y);
    current_furniture->show();
    // Ensure proper window stacking - first bring furniture to front, then character
    QTimer::singleShot(20, this, &TransparentGifViewer::raise_window_above_furniture);
#ifndef QT_NO_DEBUG
    qDebug() << "Created" << furniture_type_name << "at (" << final_x << "," << final_y << ")";
#endif

    // Set target X for character to move to
    furniture_target_x = current_furniture->get_center_x();
    // Reset walked_to_furniture flag
    walked_to_furniture = false;
    // Start walking to furniture if in auto mode
    if (auto_mode) {
        start_walk_to_furniture();
    }
}


void TransparentGifViewer::use_furniture() {
    if (!current_furniture) return;
    current_behavior = "use_furniture";
    moving = false;
    move_timer->stop();

    // Get the use point of the furniture
    QPoint target_char_pos_on_furniture = current_furniture->get_use_position();
    // Position the character at the use point
    // Character should be centered horizontally on the use point
    // and positioned so its bottom aligns with the bottom of the furniture
    int char_target_x = target_char_pos_on_furniture.x() - (width() / 2);
    int char_target_y = target_char_pos_on_furniture.y() - height(); // Character origin is bottom-left/center logic
    move(char_target_x, char_target_y);

    // Ensure character window is above furniture
    raise_window_above_furniture();

    // Use the appropriate animation based on furniture type
    QString use_type_str = current_furniture->use_type.toLower();
    if (use_type_str == "sit") {
        current_behavior = "sit";
    } else if (use_type_str == "laying") {
        current_behavior = "laying";
    } else {
        current_behavior = "wait";
    }

    if (currentAssetType == TYPE_GIF) {
        current_gif_index = gif_type_indices.value(current_behavior, gif_type_indices.value("wait",0));
        load_current_character_animation();
    } else if (currentAssetType == TYPE_SPINE) {
        QString anim_name = map_action_to_spine_animation_name(current_behavior);
        if (!anim_name.isEmpty()) spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, anim_name.toUtf8().constData(), true);
    }

    int furniture_center_x = current_furniture->get_center_x();
    facing_direction = ( (x() + width()/2) < furniture_center_x ) ? "right" : "left";

    // Different behavior based on auto mode vs manual mode
    if (auto_mode) {
        // In auto mode, use furniture for the random duration determined in auto_change_state
        // This is already set by auto_change_state, so we don't need to start another timer
        auto_timer->stop();
        int duration_min = 10000; int duration_max = 20000;
        if (!furniture_use_timer->isActive()) {
            furniture_use_timer->start(QRandomGenerator::global()->bounded(duration_min, duration_max + 1));
        }
    } else {
        // In manual mode, don't set a timer - the user will need to change state manually
        // Stop the furniture use timer if it's running
        furniture_use_timer->stop();
    }
    walked_to_furniture = false;
}


void TransparentGifViewer::finish_using_furniture() {
#ifndef QT_NO_DEBUG
    qDebug() << "Finished using furniture, removing it";
#endif
    // Remove the furniture
    if (current_furniture) {
        current_furniture->hide();
        delete current_furniture;
        current_furniture = nullptr;
    }
    // Reset the walked_to_furniture flag
    walked_to_furniture = false;
    // Explicitly stop all movement
    moving = false;
    move_timer->stop();
    // Keep the current facing direction (don't reset it)
    // Go back to wait state
    current_behavior = "wait";

    if (currentAssetType == TYPE_GIF) {
        current_gif_index = gif_type_indices.value("wait", 0);
        load_current_character_animation();
    } else if (currentAssetType == TYPE_SPINE) {
        QString anim_name = map_action_to_spine_animation_name("wait");
        if (!anim_name.isEmpty()) spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, anim_name.toUtf8().constData(), true);
    }
    if (auto_mode) {
        QTimer::singleShot(100, this, &TransparentGifViewer::auto_change_state);
    }
}


void TransparentGifViewer::toggle_menu() {
    if (menu_visible) {
        if (menu_window) menu_window->hide();
        menu_visible = false;
    } else {
        if (!menu_window) menu_window = new MenuWindow(this);
        if (menu_window) {
            // Update button text before showing
            menu_window->update_button_text();
            menu_window->update_prefix_list_selection();
            // Center relative to the main window
            menu_window->adjustSize();
            menu_window->move(x() + width() / 2 - menu_window->width() / 2,
                              y() + height() / 2 - menu_window->height() / 2);
            menu_window->show();
            menu_window->raise();
            menu_visible = true;
        }
    }
}

void TransparentGifViewer::raise_window_above_furniture() {
    // First, lower the window to reset z-order, then raise it to ensure it's at the top
    this->raise();
}

void TransparentGifViewer::handle_add_furniture(const QString& type) {
    create_furniture(type);
}


void TransparentGifViewer::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space && !auto_mode) {
        // In manual mode, if currently using furniture, pressing space should end it
        if (current_behavior == "sit" || current_behavior == "laying" || current_behavior == "use_furniture") {
            finish_using_furniture();
        } else {
            next_gif();
        }
    } else if (event->key() == Qt::Key_A) {
        toggle_auto_mode();
    } else if (event->key() == Qt::Key_M) {
        toggle_menu();
    } else if (event->key() == Qt::Key_F && !auto_mode) {
        // Manual furniture creation for testing - create a couch using default offsets
        create_furniture("couch");
        if(current_furniture) start_walk_to_furniture(); // And starts walking to it in manual mode
    } else if (event->key() == Qt::Key_T && !auto_mode) {
        // Manual furniture creation for testing - create a table using default offsets
        if (furniture_types.contains("table")) {
            create_furniture("table");
            if(current_furniture) start_walk_to_furniture(); // And starts walking to it in manual mode
        }
    } else if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}


void TransparentGifViewer::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Save complete current state
        pre_drag_auto_mode = auto_mode;
        pre_drag_moving = moving;
        pre_drag_move_direction = move_direction;
        pre_drag_facing_direction = facing_direction;
        pre_drag_behavior = current_behavior;
        // Save timer states
        pre_drag_auto_timer_active = auto_timer->isActive();

        if (currentAssetType == TYPE_GIF) pre_drag_gif_index = current_gif_index;

        // Start dragging
        dragging = true;
        drag_position = event->globalPosition().toPoint() - frameGeometry().topLeft();

        // Pause all current activity
        if (auto_mode) stop_auto_mode();
        auto_timer->stop();
        move_timer->stop();
        furniture_use_timer->stop();
        moving = false;

        // Delete furniture if it exists
        if (current_furniture) {
            current_furniture->hide();
            delete current_furniture; current_furniture = nullptr;
            walked_to_furniture = false;
        }

        // Switch to the "pick" GIF if available
        current_behavior = "pick";
        if (currentAssetType == TYPE_GIF) {
            current_gif_index = gif_type_indices.value("pick", gif_type_indices.value("wait",0));
            load_current_character_animation();
        } else if (currentAssetType == TYPE_SPINE) {
            QString anim_name = map_action_to_spine_animation_name("pick");
            if (!anim_name.isEmpty()) spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, anim_name.toUtf8().constData(), true);
        }

        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}


void TransparentGifViewer::mouseMoveEvent(QMouseEvent *event) {
    if (dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - drag_position);
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void TransparentGifViewer::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && dragging) {
        // End dragging state
        dragging = false;
        // Default to wait state after pick
        current_behavior = "wait";
        // Keep current facing direction

        if (currentAssetType == TYPE_GIF) {
            current_gif_index = gif_type_indices.value("wait",0);
            load_current_character_animation();
        } else if (currentAssetType == TYPE_SPINE) {
            QString anim_name = map_action_to_spine_animation_name("wait");
            if (!anim_name.isEmpty()) spAnimationState_setAnimationByName(currentSpineInstance.animationState, 0, anim_name.toUtf8().constData(), true);
        }
        facing_direction = pre_drag_facing_direction;

        // Restore auto mode if it was active
        if (pre_drag_auto_mode) {
            auto_mode = true;
#ifndef QT_NO_DEBUG
            qDebug() << "Auto mode restored after dragging";
#endif
            start_auto_mode();
        } else {
            auto_mode = false;
            stop_auto_mode();
        }
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

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

#include "ChibiViewer.moc"
