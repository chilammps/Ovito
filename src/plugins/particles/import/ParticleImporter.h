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

#ifndef __OVITO_PARTICLE_IMPORTER_H
#define __OVITO_PARTICLE_IMPORTER_H

#include <plugins/particles/Particles.h>
#include <core/dataset/importexport/LinkedFileImporter.h>
#include <core/utilities/io/CompressedTextReader.h>
#include <plugins/particles/data/ParticleProperty.h>
#include "ParticleImportTask.h"

namespace Ovito { namespace Plugins { namespace Particles { namespace Import {

/**
 * \brief Base class for file parsers that read particle-position data.
 */
class OVITO_PARTICLES_EXPORT ParticleImporter : public LinkedFileImporter
{
public:

	/// \brief Constructs a new instance of this class.
	ParticleImporter(DataSet* dataset) : LinkedFileImporter(dataset), _isMultiTimestepFile(false), _isNewFile(false) {
		INIT_PROPERTY_FIELD(ParticleImporter::_isMultiTimestepFile);
	}

	/// \brief Returns true if the input file contains multiple timesteps.
	bool isMultiTimestepFile() const { return _isMultiTimestepFile; }

	/// \brief Tells the importer that the input file contains multiple timesteps.
	void setMultiTimestepFile(bool enable) { _isMultiTimestepFile = enable; }

	/// \brief Scans the input source (which can be a directory or a single file) to discover all animation frames.
	virtual Future<QVector<LinkedFileImporter::FrameSourceInformation>> findFrames(const QUrl& sourceUrl) override;

	/// This method indicates whether a wildcard pattern should be automatically generated
	/// when the user picks a new input filename.
	virtual bool autoGenerateWildcardPattern() override { return !isMultiTimestepFile(); }

	/// This method is called by the LinkedFileObject each time a new source
	/// file has been selected by the user.
	virtual bool inspectNewFile(LinkedFileObject* obj) override;

protected:

	/// \brief Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// \brief Scans the given input file to find all contained simulation frames.
	virtual void scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<LinkedFileImporter::FrameSourceInformation>& frames, const QUrl& sourceUrl, CompressedTextReader& stream);

	/// Retrieves the given file in the background and scans it for simulation timesteps.
	virtual QVector<LinkedFileImporter::FrameSourceInformation> scanMultiTimestepFile(FutureInterfaceBase& futureInterface, const QUrl sourceUrl);

	/// Indicates whether the file currently being loaded has been newly selected by the user.
	/// This method should only be called from an implementation of the createImportTask() virtual method.
	bool isNewlySelectedFile() {
		if(_isNewFile) {
			_isNewFile = false;		// Reset flag after read access.
			return true;			// This is an ugly hack.
		}
		return false;
	}

private:

	/// Indicates that the input file contains multiple timesteps.
	PropertyField<bool> _isMultiTimestepFile;

	/// Flag indicating that the file currently being loaded has been newly selected by the user.
	/// If not, then the file being loaded is just another frame from the existing sequence.
	/// In this case we don't want to overwrite any settings like the periodic boundary flags that
	/// might have been changed by the user.
	bool _isNewFile;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_isMultiTimestepFile);
};

}}}}	// End of namespace

#endif // __OVITO_PARTICLE_IMPORTER_H
