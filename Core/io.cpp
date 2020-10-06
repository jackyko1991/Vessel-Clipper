#include "io.h"
#include "vtkPolyData.h"

IO::IO(QObject* parent)
{
}

IO::~IO()
{
}

bool IO::ReadSurface()
{
	std::cout << "reading surface from " << m_surfaceFile.absolutePath().toStdString() << std::endl;

	if (!(m_surfaceFile.isFile() && m_surfaceFile.exists()))
	{
		emit surfaceFileReadStatus(1);
		return 1;
	}
	else
	{
		emit surfaceFileReadStatus(0);
		return 0;
	}
}


void IO::SetSurfacePath(QString path)
{
	m_surfaceFile.setFile(path);
}