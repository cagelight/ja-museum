#pragma once

#include "Common.hh"
#include "Options.hh"

#include <QObject>

#include <memory>

struct JAPortalGeneralData {
	QString content_root;
	QString image_root;
};

struct JAPortalDataSet {
	JAPortalGeneralData const * parent;
	QString path;
	QString ident;
	QString title;
	QString desc;
	QList<int> images;
	QStringList tags;
	int64_t utime;
	int64_t mtime;
	int64_t size;
	
	bool is_complete() const;
	QString name_for_num(int, char const * ext) const;
	QList<QString> image_names(char const * ext) const;
};

struct JAPortalBackend : public QObject {
	Q_OBJECT
	
public:
	JAPortalBackend(QObject * parent);
	~JAPortalBackend();
	
	[[nodiscard]] JAPortalGeneralData & general_data();
	
	[[nodiscard]] bool has_entry(QString const &) const;
	JAPortalDataSet const & get_or_create_entry(QString const &);
	void update_entry_metadata(QMap<QString, QFileInfo> const &);
	
	[[nodiscard]] bool add_image(JAPortalDataSet const & set, QImage const & img);
	void purge_and_regenerate_images();
	
	void set_tags(JAPortalDataSet const & set, QStringList const & tags);
	void erase_tags(QStringList const & tags);
	
	void set_title(JAPortalDataSet const & set, QString const & title);
	void set_description(JAPortalDataSet const & set, QString const & desc);
	
	QStringList aggregate_tags() const;
	
public slots:
	void load();
	void save();
	void prepare(JAPortalOptions const &);
	
private:
	struct PrivateData;
	std::unique_ptr<PrivateData> m_data;
};
