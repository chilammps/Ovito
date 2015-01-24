///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OVITO_LAMMPS_DATA_IMPORTER_H
#define __OVITO_LAMMPS_DATA_IMPORTER_H

#include <plugins/particles/Particles.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <plugins/particles/import/ParticleImporter.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

/**
 * \brief File parser for LAMMPS data files.
 */
class OVITO_PARTICLES_EXPORT LAMMPSDataImporter : public ParticleImporter
{
public:

	/// \brief The LAMMPS atom_style used by the data file.
	enum LAMMPSAtomStyle {
		AtomStyle_Unknown,	//< Special value indicating that the atom_style cannot be detected and needs to be specified by the user.
		AtomStyle_Angle,
		AtomStyle_Atomic,
		AtomStyle_Body,
		AtomStyle_Bond,
		AtomStyle_Charge,
		AtomStyle_Dipole,
		AtomStyle_Electron,
		AtomStyle_Ellipsoid,
		AtomStyle_Full,
		AtomStyle_Line,
		AtomStyle_Meso,
		AtomStyle_Molecular,
		AtomStyle_Peri,
		AtomStyle_Sphere,
		AtomStyle_Template,
		AtomStyle_Tri,
		AtomStyle_Wavepacket,
		AtomStyle_Hybrid
	};
	Q_ENUMS(LAMMPSAtomStyle);

public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE LAMMPSDataImporter(DataSet* dataset) : ParticleImporter(dataset), _atomStyle(AtomStyle_Unknown) {
		INIT_PROPERTY_FIELD(LAMMPSDataImporter::_atomStyle);
	}

	/// \brief Returns the file filter that specifies the files that can be imported by this service.
	/// \return A wild-card pattern that specifies the file types that can be handled by this import class.
	virtual QString fileFilter() override { return QStringLiteral("*"); }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	virtual QString fileFilterDescription() override { return tr("LAMMPS Data Files"); }

	/// \brief Checks if the given file has format that can be read by this importer.
	virtual bool checkFileFormat(QFileDevice& input, const QUrl& sourceLocation) override;

	/// Returns the title of this object.
	virtual QString objectTitle() override { return tr("LAMMPS Data"); }

	/// This method is called by the FileSource each time a new source
	/// file has been selected by the user.
	virtual bool inspectNewFile(FileSource* obj) override;

	/// Returns the LAMMPS atom style used in the data file.
	LAMMPSAtomStyle atomStyle() const { return _atomStyle; }

	/// Specifies the LAMMPS atom style used in the data file.
	void setAtomStyle(LAMMPSAtomStyle atomStyle) { _atomStyle = atomStyle; }

	/// Displays a dialog box that allows the user to select the LAMMPS atom style of the data file.
	bool showAtomStyleDialog(QWidget* parent);

	/// Creates an asynchronous loader object that loads the data for the given frame from the external file.
	virtual std::shared_ptr<FrameLoader> createFrameLoader(const Frame& frame) override {
		return std::make_shared<LAMMPSDataImportTask>(dataset()->container(), frame, isNewlySelectedFile(), atomStyle());
	}

private:

	/// The format-specific task object that is responsible for reading an input file in the background.
	class LAMMPSDataImportTask : public ParticleFrameLoader
	{
	public:

		/// Normal constructor.
		LAMMPSDataImportTask(DataSetContainer* container, const FileSourceImporter::Frame& frame,
				bool isNewFile,
				LAMMPSAtomStyle atomStyle = AtomStyle_Unknown,
				bool detectAtomStyle = false) : ParticleFrameLoader(container, frame, isNewFile), _atomStyle(atomStyle), _detectAtomStyle(detectAtomStyle) {}

		/// Returns the LAMMPS atom style used in the data file.
		LAMMPSAtomStyle atomStyle() const { return _atomStyle; }

		/// Detects or verifies the LAMMPS atom style used by the data file.
		bool detectAtomStyle(const char* firstLine, const QByteArray& keywordLine);

	protected:

		/// Parses the given input file and stores the data in this container object.
		virtual void parseFile(CompressedTextReader& stream) override;

		/// The LAMMPS atom style used by the data file.
		LAMMPSAtomStyle _atomStyle;
		bool _detectAtomStyle;
	};

	/// The LAMMPS atom style used by the data format.
	PropertyField<LAMMPSAtomStyle, int> _atomStyle;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_atomStyle);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Particles::LAMMPSDataImporter::LAMMPSAtomStyle);
Q_DECLARE_TYPEINFO(Ovito::Particles::LAMMPSDataImporter::LAMMPSAtomStyle, Q_PRIMITIVE_TYPE);

#endif // __OVITO_LAMMPS_DATA_IMPORTER_H
