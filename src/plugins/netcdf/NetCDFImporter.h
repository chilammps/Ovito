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

#ifndef __OVITO_NETCDF_IMPORTER_H
#define __OVITO_NETCDF_IMPORTER_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <plugins/particles/import/InputColumnMappingDialog.h>
#include <plugins/particles/import/ParticleImporter.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

/**
 * \brief File parser for NetCDF simulation files.
 */
class NetCDFImporter : public ParticleImporter
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE NetCDFImporter(DataSet *dataset) : ParticleImporter(dataset), _useCustomColumnMapping(false) {
		INIT_PROPERTY_FIELD(NetCDFImporter::_useCustomColumnMapping);
		setMultiTimestepFile(true);
	}

	/// \brief Returns the file filter that specifies the files that can be imported by this service.
	/// \return A wild-card pattern that specifies the file types that can be handled by this import class.
	virtual QString fileFilter() override { return "*"; }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	virtual QString fileFilterDescription() override { return tr("NetCDF Files"); }

	/// \brief Checks if the given file has format that can be read by this importer.
	virtual bool checkFileFormat(QFileDevice& input, const QUrl& sourceLocation) override;

	/// Returns the title of this object.
	virtual QString objectTitle() override { return tr("NetCDF"); }

	/// \brief Returns the user-defined mapping between data columns in the input file and
	///        the internal particle properties.
	const InputColumnMapping& customColumnMapping() const { return _customColumnMapping; }

	/// \brief Sets the user-defined mapping between data columns in the input file and
	///        the internal particle properties.
	void setCustomColumnMapping(const InputColumnMapping& mapping);

	/// Returns whether the mapping between input file columns and particle
	/// properties is done automatically or by the user.
	bool useCustomColumnMapping() const { return _useCustomColumnMapping; }

	/// Sets whether the mapping between input file columns and particle
	/// properties is done automatically or by the user.
	void setUseCustomColumnMapping(bool useCustomMapping) { _useCustomColumnMapping = useCustomMapping; }

	/// Displays a dialog box that allows the user to edit the custom file column to particle
	/// property mapping.
	void showEditColumnMappingDialog(QWidget* parent = nullptr);

	/// Creates an asynchronous loader object that loads the data for the given frame from the external file.
	virtual std::shared_ptr<FrameLoader> createFrameLoader(const Frame& frame) override {
		return std::make_shared<NetCDFImportTask>(dataset()->container(), frame, isNewlySelectedFile(), _useCustomColumnMapping, _customColumnMapping);
	}

private:

	/// The format-specific task object that is responsible for reading an input file in the background.
	class NetCDFImportTask : public ParticleFrameLoader
	{
	public:

		/// Normal constructor.
		NetCDFImportTask(DataSetContainer* container, const FileSourceImporter::Frame& frame, bool isNewFile,
				bool useCustomColumnMapping, const InputColumnMapping& customColumnMapping)
			: ParticleFrameLoader(container, frame, isNewFile), _parseFileHeaderOnly(false), _useCustomColumnMapping(useCustomColumnMapping), _customColumnMapping(customColumnMapping), _ncIsOpen(false), _ncid(-1) {}

		/// Constructor used when reading only the file header information.
		NetCDFImportTask(DataSetContainer* container, const FileSourceImporter::Frame& frame)
			: ParticleFrameLoader(container, frame, true), _parseFileHeaderOnly(true), _useCustomColumnMapping(false), _ncIsOpen(false), _ncid(-1) {}

		/// Returns the file column mapping used to load the file.
		const InputColumnMapping& columnMapping() const { return _customColumnMapping; }

	protected:

        /// Map dimensions from NetCDF file to internal representation.
        void detectDims(int movieFrame, int particleCount, int nDims, int *dimIds, int &nDimsDetected, int &componentCount, int &nativeComponentCount, size_t *startp, size_t *countp);

		/// Parses the given input file and stores the data in this container object.
		virtual void parseFile(CompressedTextReader& stream) override;

	private:

		/// Is the NetCDF file open?
		bool _ncIsOpen;

		/// NetCDF ids.
		int _ncid, _frame_dim, _atom_dim, _spatial_dim, _Voigt_dim;
		int _cell_spatial_dim, _cell_angular_dim;
		int _cell_origin_var, _cell_lengths_var, _cell_angles_var;
		int _shear_dx_var;

		/// Open NetCDF file, and load additional information
		void openNetCDF(const QString &filename);

		/// Close the current NetCDF file.
		void closeNetCDF();

		bool _parseFileHeaderOnly;
		bool _useCustomColumnMapping;
		InputColumnMapping _customColumnMapping;
	};

protected:

	/// \brief Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// \brief Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// \brief Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// \brief Scans the given input file to find all contained simulation frames.
	virtual void scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<FileSourceImporter::Frame>& frames, const QUrl& sourceUrl, CompressedTextReader& stream) override;

	/// \brief Guesses the mapping of an input file field to one of OVITO's internal particle properties.
	static InputColumnInfo mapVariableToColumn(const QString& name, int dataType);

private:

	/// Controls whether the mapping between input file columns and particle
	/// properties is done automatically or by the user.
	PropertyField<bool> _useCustomColumnMapping;

	/// Stores the user-defined mapping between data columns in the input file and
	/// the internal particle properties.
	InputColumnMapping _customColumnMapping;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_useCustomColumnMapping);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A properties editor for the NetCDFImporter class.
 */
class NetCDFImporterEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE NetCDFImporterEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

protected Q_SLOTS:

	/// Is called when the user pressed the "Edit column mapping" button.
	void onEditColumnMapping();

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_NETCDF_IMPORTER_H
