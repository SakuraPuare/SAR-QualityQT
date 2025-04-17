#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <cmath>
#include <functional> // For std::bind or lambda used in constructor
#include <vector>

#include <QDateTime> // Needed for logMessage timestamp
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
// #include <QImage> // No longer needed directly here
#include <QCoreApplication> // For processEvents
#include <QFileInfo>        // For QFileInfo
#include <QMessageBox>
#include <QMimeData>
#include <QPixmap>
#include <QUrl>

// Keep OpenCV includes needed for analysis functions if they remain in this
// file
#include <opencv2/core.hpp> // Needed for cv::Mat type passed to analysis funcs

// Keep GDAL includes only for GDALAllRegister
#include "gdal_priv.h" // For GDALAllRegister, GDALClose(if needed), GDALDataTypeIsComplex etc. might be needed by analysis functions if not refactored.

// Include analysis utilities header
#include "analysis_utils.h"

// Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
      // Initialize ImageHandler with a lambda that captures 'this' and calls
      // logMessage
      ,
      m_imageHandler([this](const QString &msg) { this->logMessage(msg); }) {
  ui->setupUi(this);
  setAcceptDrops(true); // Enable drag & drop
  GDALAllRegister();    // Initialize GDAL drivers
  logMessage(tr("Application started. GDAL initialized. Drag & Drop enabled."));

  // Initial UI State
  ui->startAnalysisButton->setEnabled(false);
  on_checkBoxSelectAll_toggled(ui->checkBoxSelectAll->isChecked());
  ui->valueFilename->setText(tr("N/A"));
  ui->valueDimensions->setText(tr("N/A"));
  ui->valueDataType->setText(tr("N/A"));
  ui->imageDisplayLabel->setText(tr("Image Display Area"));
  ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
  ui->imageDisplayLabel->setStyleSheet("QLabel { color: grey; }");

  // Placeholder texts (no change)
  ui->overviewResultsTextEdit->setPlaceholderText(
      tr("Summary of all selected analysis results will appear here..."));
  ui->method1ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for SNR/ENL Analysis..."));
  ui->method2ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Information Content (Entropy)..."));
  ui->method3ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Clarity (Gradient Magnitude)..."));
  ui->method4ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Radiometric Stats (Min, Max, Mean, StdDev)..."));
  ui->method5ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for GLCM Texture Features..."));
  ui->logTextEdit->setPlaceholderText(
      tr("Log messages (loading, analysis steps, errors) will appear here..."));
}

// Destructor
MainWindow::~MainWindow() {
  // No need to call closeCurrentImage() here.
  // m_imageHandler's destructor will be called automatically,
  // which in turn calls its own closeImage() method.
  delete ui;
  // Consider GDALCleanupAll(); if appropriate for application lifecycle
}

// Log Message (no change)
void MainWindow::logMessage(const QString &message) {
  QString timestamp =
      QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
  ui->logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
}

// Close Current Image (Handles UI reset)
void MainWindow::closeCurrentImage() {
  logMessage(tr("Closing current image and resetting UI."));
  m_imageHandler.closeImage(); // Delegate resource closing to handler

  // Reset UI elements to initial state
  ui->valueFilename->setText(tr("N/A"));
  ui->valueDimensions->setText(tr("N/A"));
  ui->valueDataType->setText(tr("N/A"));

  ui->imageDisplayLabel->clear(); // Clear the pixmap
  ui->imageDisplayLabel->setText(
      tr("Image Display Area")); // Restore placeholder
  ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
  ui->imageDisplayLabel->setStyleSheet(
      "QLabel { color: grey; }"); // Restore style

  ui->startAnalysisButton->setEnabled(false); // Disable analysis

  // Clear results tabs
  ui->overviewResultsTextEdit->clear();
  ui->method1ResultsTextEdit->clear();
  ui->method2ResultsTextEdit->clear();
  ui->method3ResultsTextEdit->clear();
  ui->method4ResultsTextEdit->clear();
  ui->method5ResultsTextEdit->clear();
}

// Drag Enter Event (Minor logging change)
void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
      QString filePath = urls.first().toLocalFile();
      QFileInfo fileInfo(filePath);
      QString suffix = fileInfo.suffix().toLower();
      QStringList supportedSuffixes = {"tif", "tiff", "img", "hdr", "dat"};
      if (supportedSuffixes.contains(suffix)) {
        event->acceptProposedAction();
        // Log handled internally by openImageFile/ImageHandler now,
        // but we can keep this high-level log.
        logMessage(tr("Drag entered with supported file: %1")
                       .arg(fileInfo.fileName()));
        return;
      } else {
        logMessage(
            tr("Drag entered with unsupported file type: .%1").arg(suffix));
      }
    }
  }
  event->ignore();
}

// Drop Event (Calls modified openImageFile)
void MainWindow::dropEvent(QDropEvent *event) {
  const QMimeData *mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    QList<QUrl> urls = mimeData->urls();
    if (!urls.isEmpty()) {
      QString filePath = urls.first().toLocalFile();
      logMessage(tr("File dropped: %1").arg(filePath));
      openImageFile(filePath); // Call the updated file opening function
      event->acceptProposedAction();
      return;
    }
  }
  event->ignore();
}

// Open Image File (Uses ImageHandler)
void MainWindow::openImageFile(const QString &filePath) {
  if (filePath.isEmpty()) {
    logMessage(tr("Image opening cancelled or invalid path provided."));
    return;
  }

  logMessage(tr("Attempting to open image via MainWindow: %1").arg(filePath));
  QCoreApplication::processEvents(); // Update UI to show log message

  // Use ImageHandler to load the image
  bool success = m_imageHandler.loadImage(filePath);

  if (success) {
    logMessage(tr("Image loaded successfully by ImageHandler: %1")
                   .arg(m_imageHandler.getFilename()));

    // Update UI with info from ImageHandler
    ui->valueFilename->setText(m_imageHandler.getFilename());
    ui->valueDimensions->setText(m_imageHandler.getDimensionsString());
    ui->valueDataType->setText(m_imageHandler.getDataTypeString());

    // Get and display the pixmap from ImageHandler
    QPixmap pixmap =
        m_imageHandler.getDisplayPixmap(ui->imageDisplayLabel->size());
    if (!pixmap.isNull()) {
      ui->imageDisplayLabel->setPixmap(pixmap);
      ui->imageDisplayLabel->setAlignment(Qt::AlignCenter); // Ensure centering
      ui->imageDisplayLabel->setStyleSheet(""); // Clear placeholder style
      logMessage(tr("Image successfully displayed."));
      ui->startAnalysisButton->setEnabled(true); // Enable analysis
    } else {
      logMessage(tr("Error: ImageHandler provided a null QPixmap."));
      // Show error in display label
      ui->imageDisplayLabel->setText(tr("Error: Display failed"));
      ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
      ui->imageDisplayLabel->setStyleSheet("QLabel { color: red; }");
      closeCurrentImage(); // Reset UI as display failed
      QMessageBox::critical(
          this, tr("Display Error"),
          tr("Failed to prepare image for display after loading."));
    }
  } else {
    // loadImage failed, ImageHandler already logged the GDAL error
    logMessage(tr("ImageHandler failed to load the image."));
    QMessageBox::critical(
        this, tr("Image Load Error"),
        tr("Could not open or read the selected image file.\nPath: %1\nPlease "
           "check file integrity, permissions, and see logs for details.")
            .arg(filePath));
    closeCurrentImage(); // Ensure UI is reset after failure
  }
}

// --- displayImage method is removed ---
// --- updateImageInfo method is removed ---

// Start Analysis Button Clicked (MODIFIED)
void MainWindow::on_startAnalysisButton_clicked() {
  if (!m_imageHandler.isValid()) {
    QMessageBox::warning(
        this, tr("Analysis Not Started"),
        tr("Please open a valid image file before starting the analysis."));
    logMessage(tr("Analysis button clicked, but no valid image is loaded."));
    return;
  }

  logMessage(tr("Analysis process started by user."));
  ui->progressBar->setValue(0);

  ui->overviewResultsTextEdit->clear();
  ui->method1ResultsTextEdit->clear();
  ui->method2ResultsTextEdit->clear();
  ui->method3ResultsTextEdit->clear();
  ui->method4ResultsTextEdit->clear();
  ui->method5ResultsTextEdit->clear();

  // --- Get image data from ImageHandler ---
  const cv::Mat &imageData = m_imageHandler.getImageData();

  // --- Define analysis tasks using standalone functions ---
  int totalSteps = 0;
  // Store pairs of { Task Function, Result UI Update Function }
  std::vector<std::pair<std::function<AnalysisResult()>,
                        std::function<void(const AnalysisResult &)>>>
      analysisPipeline;
  std::vector<QWidget *> resultTabs; // Keep track of tabs to switch focus

  // Capture necessary variables for lambdas
  auto updateUI = [this](QTextEdit *detailEdit, const AnalysisResult &res) {
    logMessage(tr("Analysis '%1' finished. Success: %2")
                   .arg(res.analysisName)
                   .arg(res.success ? "Yes" : "No"));
    // Log the internal details first
    if (!res.detailedLog.contains(
            "Internal Log:")) { // Avoid double logging if internal log is
                                // already part of detailed log
      logMessage(
          tr("Details for %1:\n%2").arg(res.analysisName, res.detailedLog));
    } else {
      logMessage(tr("Detailed log for %1 already contains internal messages.")
                     .arg(res.analysisName));
      // Optional: Extract and log only the 'Internal Log' parts if desired
    }

    detailEdit->setText(res.detailedLog); // Display full log in details tab
    ui->overviewResultsTextEdit->append(
        res.overviewSummary); // Append summary to overview
  };

  if (ui->checkBoxSNR->isChecked()) {
    totalSteps++;
    analysisPipeline.push_back(
        {// Task function: Call standalone analysis
         [&imageData]() { return performSNRAnalysis(imageData); },
         // UI update function: Update relevant UI elements
         [this, updateUI](const AnalysisResult &res) {
           updateUI(ui->method1ResultsTextEdit, res);
         }});
    resultTabs.push_back(ui->tabMethod1);
  }
  if (ui->checkBoxInfoContent->isChecked()) {
    totalSteps++;
    analysisPipeline.push_back(
        {[&imageData]() { return performInfoContentAnalysis(imageData); },
         [this, updateUI](const AnalysisResult &res) {
           updateUI(ui->method2ResultsTextEdit, res);
         }});
    resultTabs.push_back(ui->tabMethod2);
  }
  if (ui->checkBoxClarity->isChecked()) {
    totalSteps++;
    analysisPipeline.push_back(
        {[&imageData]() { return performClarityAnalysis(imageData); },
         [this, updateUI](const AnalysisResult &res) {
           updateUI(ui->method3ResultsTextEdit, res);
         }});
    resultTabs.push_back(ui->tabMethod3);
  }
  if (ui->checkBoxRadiometricAccuracy->isChecked()) {
    totalSteps++;
    analysisPipeline.push_back(
        {[&imageData]() { return performRadiometricAnalysis(imageData); },
         [this, updateUI](const AnalysisResult &res) {
           updateUI(ui->method4ResultsTextEdit, res);
         }});
    resultTabs.push_back(ui->tabMethod4);
  }
  if (ui->checkBoxGLCM->isChecked()) {
    totalSteps++;
    analysisPipeline.push_back(
        {[&imageData]() { return performGLCMAnalysis(imageData); },
         [this, updateUI](const AnalysisResult &res) {
           updateUI(ui->method5ResultsTextEdit, res);
         }});
    resultTabs.push_back(ui->tabMethod5);
  }
  // ... Add other analysis methods similarly ...

  if (totalSteps == 0) {
    QMessageBox::information(this, tr("No Analysis Selected"),
                             tr("Please select at least one analysis method."));
    logMessage(tr("Analysis stopped: No methods were selected."));
    return;
  }

  ui->progressBar->setMaximum(totalSteps);
  ui->overviewResultsTextEdit->setText(tr("Starting selected analyses...\n"));

  int currentStep = 0;
  for (size_t i = 0; i < analysisPipeline.size(); ++i) {
    logMessage(tr("Performing analysis step %1 of %2...")
                   .arg(currentStep + 1)
                   .arg(totalSteps));
    QCoreApplication::processEvents(); // Update UI before starting analysis

    AnalysisResult result =
        analysisPipeline[i].first();    // Execute analysis task
    analysisPipeline[i].second(result); // Update UI with results

    currentStep++;
    ui->progressBar->setValue(currentStep);
    if (resultTabs[i]) {
      ui->resultsTabWidget->setCurrentWidget(resultTabs[i]);
    }
    QCoreApplication::processEvents(); // Update UI after analysis
  }

  logMessage(tr("All selected analyses finished."));
  ui->overviewResultsTextEdit->append(
      tr("\nAnalysis complete. Check individual tabs for detailed results."));
  ui->resultsTabWidget->setCurrentWidget(ui->tabOverview);

  QMessageBox::information(this, tr("Analysis Complete"),
                           tr("Selected image analyses have finished."));
}

// Checkbox Select All (no change)
void MainWindow::on_checkBoxSelectAll_toggled(bool checked) {
  ui->checkBoxSNR->setChecked(checked);
  ui->checkBoxInfoContent->setChecked(checked);
  ui->checkBoxClarity->setChecked(checked);
  ui->checkBoxRadiometricAccuracy->setChecked(checked);
  ui->checkBoxGLCM->setChecked(checked);
  logMessage(checked ? tr("Selected all analysis methods.")
                     : tr("Deselected all analysis methods."));
}

// Menu Action: Open Image (Calls modified openImageFile)
void MainWindow::on_actionOpenImage_triggered() {
  QString filePath =
      QFileDialog::getOpenFileName(this, tr("Open SAR Image File"), QString(),
                                   tr("Supported Image Formats (*.tif *.tiff "
                                      "*.img *.hdr *.dat);;All Files (*.*)"));

  if (!filePath.isEmpty()) {
    logMessage(tr("Opening image via File menu: %1").arg(filePath));
    openImageFile(filePath); // Call the updated file opening function
  } else {
    logMessage(tr("Image opening via File menu cancelled by user."));
  }
}

// --- Member function definitions for analysis are removed from here ---
// --- They now exist as standalone functions in analysis_*.cpp files ---
