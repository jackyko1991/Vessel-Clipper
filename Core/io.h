#ifndef IO_H
#define IO_H

// qt
#include <QObject>
#include <QFileInfo>

// vtk


class IO : public QObject
{
	Q_OBJECT

public:
	explicit IO(QObject* parent = 0);
	~IO();
	void SetSurfacePath(QString path);
	bool ReadSurface();
	void ReadCenterline();

signals:
	// 0 for success, 1 for fail
	void surfaceFileReadStatus(bool);
	//void targetFileReadStatus(bool);
	//void transformedFileSaveStatus(bool);

private:
	QFileInfo m_surfaceFile;


};

#endif // ! IO_H
