#pragma once

#include "Options.hh"

#include <QLabel>
#include <QScrollArea>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>

class ClickImage : public QLabel {
	Q_OBJECT
public:
	ClickImage(QPixmap const & pix, QWidget * parent);
signals:
	void clicked();
protected:
	void mouseDoubleClickEvent(QMouseEvent *) override;
};

class ImagePaneItem : public QWidget {
	Q_OBJECT
	
public:
	ImagePaneItem(QString ipath, QString tpath, QWidget * parent);
	~ImagePaneItem();
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
	
	QString m_ipath;
	QString m_tpath;
	QPixmap m_thumb;
};

class ImagePane : public QScrollArea {
	Q_OBJECT
	
public:
	ImagePane(QWidget * parent);
	~ImagePane();
	
	void add_image(QString iname, QString tname);
	
signals:
	void image_dropped(QString);
	
public slots:
	void clear();
	void prepare(JAPortalOptions const &);
	
protected:
	inline void dragEnterEvent(QDragEnterEvent * event) override { event->acceptProposedAction(); }
	inline void dragMoveEvent(QDragMoveEvent * event) override { event->acceptProposedAction(); }
	inline void dragLeaveEvent(QDragLeaveEvent * event) override { event->accept(); }
	void dropEvent(QDropEvent *) override;
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
	JAPortalOptions m_opt;
};
