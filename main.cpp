#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo> // For Qt standard translations
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // --- i18n Setup ---
    QTranslator qtTranslator; // Translator for Qt standard texts (e.g., "OK", "Cancel")
    QTranslator appTranslator; // Translator for application texts

    // Detect system locale
    QLocale locale = QLocale::system();
    qDebug() << "System locale detected:" << locale.name(); // e.g., "zh_CN", "en_US"

    // Load Qt standard translations for the current locale
    QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
     qDebug() << "Looking for Qt standard translations in:" << qtTranslationsPath;
    if (qtTranslator.load(locale, "qtbase", "_", qtTranslationsPath)) {
         qDebug() << "Loaded Qt standard translations:" << qtTranslator.filePath();
        a.installTranslator(&qtTranslator);
    } else {
        qWarning() << "Failed to load Qt standard translations for locale" << locale.name();
         // Try loading generic Qt translations as fallback
        if (qtTranslator.load(locale, "qt", "_", qtTranslationsPath)) {
            qDebug() << "Loaded generic Qt translations:" << qtTranslator.filePath();
            a.installTranslator(&qtTranslator);
        } else {
             qWarning() << "Failed to load generic Qt translations for locale" << locale.name();
        }
    }


    // Load application translations from Qt Resource System
    // The path corresponds to the prefix and alias in translations.qrc
    QString appTranslationFile = QString(":/i18n/SAR-QualityQT_%1.qm").arg(locale.name());
    qDebug() << "Looking for application translation file:" << appTranslationFile;
    if (appTranslator.load(appTranslationFile)) {
        qDebug() << "Loaded application translation:" << appTranslationFile;
        a.installTranslator(&appTranslator);
    } else {
        qWarning() << "Failed to load application translation file:" << appTranslationFile;
        // Fallback to generic language if specific locale (e.g., en_US) fails
        QString genericLang = locale.name().split('_').first(); // e.g., "en" from "en_US"
        appTranslationFile = QString(":/i18n/SAR-QualityQT_%1.qm").arg(genericLang);
         qDebug() << "Looking for generic application translation file:" << appTranslationFile;
        if (appTranslator.load(appTranslationFile)) {
             qDebug() << "Loaded generic application translation:" << appTranslationFile;
            a.installTranslator(&appTranslator);
        } else {
             qWarning() << "Failed to load generic application translation file:" << appTranslationFile;
        }
    }
    // --- End i18n Setup ---


    MainWindow w;
    w.show();
    return a.exec();
}
