#include "ImagePane.hh"
#include "Common.hh"

#include <QLabel>
#include <QScrollBar>

#include <QGridLayout>
#include <QHBoxLayout>

#include <QDesktopServices>
#include <QMouseEvent>
#include <QDropEvent>
#include <QMimeData>

ClickImage::ClickImage(QPixmap const & pix, QWidget * parent) : QLabel { parent } {
	setPixmap(pix);
}

void ClickImage::mouseDoubleClickEvent(QMouseEvent * evt) {
	evt->accept();
	emit clicked();
}

struct ImagePaneItem::PrivateData {
	
};

ImagePaneItem::ImagePaneItem(QString ipath, QString tpath, QWidget * parent) : QWidget { parent }  {
	m_ipath = ipath;
	m_tpath = tpath;
	m_thumb = QPixmap { m_tpath };
	
	Q_ASSERT(!m_thumb.isNull());
	
	QGridLayout * layout = new QGridLayout { this };
	layout->setMargin(0);
	layout->setSpacing(0);
	ClickImage * img_label = new ClickImage { m_thumb, this };
	layout->addWidget(img_label, 0, 0);
	
	connect(img_label, &ClickImage::clicked, this, [this](){
		QDesktopServices::openUrl(QUrl::fromLocalFile(m_ipath));
	});
	
	setMinimumSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
	setMaximumSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

ImagePaneItem::~ImagePaneItem() = default;

struct ImagePane::PrivateData {
	QList<ImagePaneItem *> items;
	QHBoxLayout * layout = nullptr;
};

ImagePane::ImagePane(QWidget* parent) : QScrollArea { parent }, m_data { new PrivateData } {
	
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setWidgetResizable(true);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAcceptDrops(true);
	
	QWidget * content_wid = new QWidget { this };
	
	m_data->layout = new QHBoxLayout { content_wid };
	m_data->layout->setMargin(0);
	m_data->layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	
	setWidget(content_wid);
	setMinimumHeight(horizontalScrollBar()->height() + THUMBNAIL_HEIGHT);
}

ImagePane::~ImagePane() = default;

void ImagePane::add_image(QString iname, QString tname) {
	QString ipath = m_opt.output_dir.absolutePath() + QDir::separator() + IMAGE_FULL_PATH + QDir::separator() + iname;
	QString tpath = m_opt.output_dir.absolutePath() + QDir::separator() + IMAGE_THUMB_PATH + QDir::separator() + tname;
	ImagePaneItem * item = new ImagePaneItem { ipath, tpath, this };
	m_data->items.append(item);
	m_data->layout->addWidget(item);
}

void ImagePane::clear() {
	for (auto * item : m_data->items) delete item;
	m_data->items.clear();
}

void ImagePane::prepare(JAPortalOptions const & opt) {
	clear();
	m_opt = opt;
}

void ImagePane::dropEvent(QDropEvent * evt) {
	QMimeData const * mime = evt->mimeData();
	if (!mime->hasUrls()) return;
	for (QUrl const & url : mime->urls()) {
		if (!url.isLocalFile()) continue;
		emit image_dropped(url.toLocalFile());
	}
}
