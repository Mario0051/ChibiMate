#include <spine/extension.h>
#include <QFile>
#include <QImage>
#include <QDebug>
#include <cstdlib>
#include <cstring>

void* _malloc (size_t size, const char* file, int line) {
    (void)file;
    (void)line;
    return std::malloc(size);
}

void* _calloc (size_t num, size_t size, const char* file, int line) {
    (void)file;
    (void)line;
    return std::calloc(num, size);
}

void _free (void* ptr) {
    std::free(ptr);
}

void _spAtlasPage_createTexture (spAtlasPage* self, const char* path) {
    QImage* image = new QImage();
    if (image->load(QString::fromUtf8(path))) {
        self->rendererObject = image;
        self->width = image->width();
        self->height = image->height();
#ifndef QT_NO_DEBUG
        qDebug() << "Spine: Loaded texture" << path << "(" << self->width << "x" << self->height << ")";
#endif
    } else {
        qWarning() << "Spine: Failed to load texture" << QString::fromUtf8(path);
        delete image;
        self->rendererObject = nullptr;
    }
}

void _spAtlasPage_disposeTexture (spAtlasPage* self) {
    if (self->rendererObject) {
        delete static_cast<QImage*>(self->rendererObject);
        self->rendererObject = nullptr;
    }
}

char* _spUtil_readFile (const char* path, int* length) {
    QFile file(QString::fromUtf8(path));
    if (!file.open(QIODevice::ReadOnly)) {
        *length = 0;
        qWarning() << "Spine: Could not open file:" << QString::fromUtf8(path);
        return nullptr;
    }
    QByteArray data = file.readAll();
    file.close();

    *length = data.size();
    if (*length == 0) {
        char* emptyBuffer = (char*)_malloc(1, __FILE__, __LINE__);
        if (emptyBuffer) {
            emptyBuffer[0] = '\0';
        }
        return emptyBuffer;
    }

    char* buffer = (char*)_malloc(*length + 1, __FILE__, __LINE__);
    if (!buffer) {
        *length = 0;
        qWarning() << "Spine: Failed to allocate buffer for file" << QString::fromUtf8(path);
        return nullptr;
    }
    std::memcpy(buffer, data.constData(), *length);
    buffer[*length] = '\0';
    return buffer;
}
