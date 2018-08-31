#ifndef ENUMS_H
#define ENUMS_H

#include <QtCore>

class Enums {
public:
    enum ImportExportMode {
        Import,
        Export,
    };
    Q_ENUM(ImportExportMode)
    Q_GADGET
    Enums() = delete; // std=c++11, обеспечивает запрет на создание любого экземпляра Enums
};

#endif // ENUMS_H
