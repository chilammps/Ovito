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

#include <core/Core.h>
#include "OutputColumnMapping.h"

namespace Viz {

/******************************************************************************
 * Inserts a column that will be written to the output data file.
 *****************************************************************************/
void OutputColumnMapping::insertColumn(int columnIndex, DataChannel::DataChannelIdentifier channelId, const QString& channelName, size_t vectorComponent)
{
	OVITO_ASSERT(columnIndex >= 0);

	// Expand column array if necessary and initialize all new columns to their default values.
	while(columnIndex >= columnCount()) {
		MapEntry newEntry;
		newEntry.dataChannelId = DataChannel::UserDataChannel;
		newEntry.vectorComponent = 0;
		columns.append(newEntry);
	}

	columns[columnIndex].dataChannelId = channelId;
	columns[columnIndex].dataChannelName = channelName;
	columns[columnIndex].vectorComponent = vectorComponent;
}

/******************************************************************************
 * Removes the definition of a column.
 *****************************************************************************/
void OutputColumnMapping::removeColumn(int columnIndex)
{
	OVITO_ASSERT(columnIndex >= 0);
	if(columnIndex < columnCount()) {
		columns.remove(columnIndex);
	}
}

/******************************************************************************
 * Saves the mapping to the given stream.
 *****************************************************************************/
void OutputColumnMapping::saveToStream(SaveStream& stream) const
{
	stream.beginChunk(0x10000000);
	stream << columns.size();
	for(QVector<MapEntry>::const_iterator entry = columns.constBegin(); entry != columns.constEnd(); ++entry) {
		stream.writeEnum(entry->dataChannelId);
		stream << entry->dataChannelName;
		stream.writeSizeT(entry->vectorComponent);
	}
	stream.endChunk();
}

/******************************************************************************
 * Loads the mapping from the given stream.
 *****************************************************************************/
void OutputColumnMapping::loadFromStream(LoadStream& stream)
{
	stream.expectChunk(0x10000000);
	int numColumns;
	stream >> numColumns;
	columns.resize(numColumns);
	for(QVector<MapEntry>::iterator entry = columns.begin(); entry != columns.end(); ++entry) {
		stream.readEnum(entry->dataChannelId);
		stream >> entry->dataChannelName;
		stream.readSizeT(entry->vectorComponent);
	}
	stream.closeChunk();
}

/******************************************************************************
 * Saves the mapping into a byte array.
 *****************************************************************************/
QByteArray OutputColumnMapping::toByteArray() const
{
	QByteArray buffer;
	QDataStream dstream(&buffer, QIODevice::WriteOnly);
	SaveStream stream(dstream);
	saveToStream(stream);
	stream.close();
	return buffer;
}

/******************************************************************************
 * Loads the mapping from a byte array.
 *****************************************************************************/
void OutputColumnMapping::fromByteArray(const QByteArray& array)
{
	QDataStream dstream(array);
	LoadStream stream(dstream);
	loadFromStream(stream);
	stream.close();
}

/******************************************************************************
 * Saves the mapping the application's settings store.
 *****************************************************************************/
void OutputColumnMapping::savePreset(const QString& presetName) const
{
	QSettings settings;
	settings.beginGroup("atomviz/io/channelmapping/presets");
	settings.beginGroup(presetName);
	settings.setValue("name", presetName);
	settings.setValue("data", toByteArray());
	settings.endGroup();
	settings.endGroup();
}

/******************************************************************************
 * Loads a mapping from the application's settings store.
 *****************************************************************************/
void OutputColumnMapping::loadPreset(const QString& presetName)
{
	QSettings settings;
	settings.beginGroup("atomviz/io/channelmapping/presets");
	settings.beginGroup(presetName);
	if(settings.value("name").toString() != presetName)
		throw Exception(tr("No preset found with the name: %1").arg(presetName));
	fromByteArray(settings.value("data").toByteArray());
}

/******************************************************************************
 * Returns a list of all presets found in the
 *****************************************************************************/
QStringList OutputColumnMapping::listPresets()
{
	QStringList list;
	QSettings settings;
	settings.beginGroup("atomviz/io/channelmapping/presets");
	// Find preset with the given name.
	Q_FOREACH(QString group, settings.childGroups()) {
		settings.beginGroup(group);
		list.push_back(settings.value("name").toString());
		settings.endGroup();
	}
	return list;
}

/******************************************************************************
 * Deletes a mapping from the application's settings store.
 *****************************************************************************/
void OutputColumnMapping::deletePreset(const QString& presetName)
{
	QSettings settings;
	settings.beginGroup("atomviz/io/channelmapping/presets");
	// Find preset with the given name.
	Q_FOREACH(QString group, settings.childGroups()) {
		settings.beginGroup(group);
		if(settings.value("name").toString() == presetName) {
			settings.endGroup();
			settings.remove(group);
			return;
		}
		settings.endGroup();
	}
	throw Exception(tr("No preset found with the name: %1").arg(presetName));
}

/******************************************************************************
 * Makes a copy of the mapping object.
 *****************************************************************************/
OutputColumnMapping& OutputColumnMapping::operator=(const OutputColumnMapping& other)
{
	this->columns = other.columns;
	return *this;
}

/******************************************************************************
 * Initializes the helper object.
 *****************************************************************************/
DataRecordWriterHelper::DataRecordWriterHelper(const OutputColumnMapping* mapping, AtomsObject* source)
{
	CHECK_POINTER(mapping);
	CHECK_OBJECT_POINTER(source);

	this->mapping = mapping;
	this->source = source;

	// Gather the source data channels.
	for(int i=0; i<mapping->columnCount(); i++) {

		DataChannel::DataChannelIdentifier channelId = mapping->getChannelId(i);
		QString channelName = mapping->getChannelName(i);
		size_t vectorComponent = mapping->getVectorComponent(i);

		DataChannel* channel;
		if(channelId != DataChannel::UserDataChannel)
			channel = source->getStandardDataChannel(channelId);
		else
			channel = source->findDataChannelByName(channelName);

		// Validate column
		if(channel == NULL && channelId != DataChannel::AtomIndexChannel) {
			throw Exception(tr("The mapping between data channels and columns in the output file is not valid. "
			                   "The source dataset does not contain a data channel named '%1'.").arg(channelName));
		}
		if(channel && channel->componentCount() <= vectorComponent)
			throw Exception(tr("The vector component specified for column %1 exceeds the number of available vector components in data channel '%2'.").arg(i).arg(channelName));
		if(channel && channel->type() == QMetaType::Void)
			throw Exception(tr("The data channel '%1' cannot be written to the output file because it is empty.").arg(channelName));

		// Build internal list of channel objects for fast look up during writing.
		channels.push_back(channel);
		vectorComponents.push_back(vectorComponent);
	}
}

/******************************************************************************
 * Writes the data record for a single atom to the output stream.
 *****************************************************************************/
void DataRecordWriterHelper::writeAtom(int atomIndex, QIODevice& stream)
{
	QVector<DataChannel*>::const_iterator channel = channels.constBegin();
	QVector<size_t>::const_iterator vcomp = vectorComponents.constBegin();
	for(; channel != channels.constEnd(); ++channel, ++vcomp) {
		if(channel != channels.constBegin()) stream.putChar(' ');
		if(*channel != NULL) {
			if((*channel)->type() == qMetaTypeId<int>())
				buffer.setNum((*channel)->getIntComponent(atomIndex, *vcomp));
			else if((*channel)->type() == qMetaTypeId<FloatType>())
				buffer.setNum((*channel)->getFloatComponent(atomIndex, *vcomp), 'g', 12);
			else buffer.clear();
		}
		else {
			buffer.setNum(atomIndex+1);
		}
		stream.write(buffer);
	}
}

/******************************************************************************
 * Writes the data record for a single atom to the output stream.
 *****************************************************************************/
void DataRecordWriterHelper::writeAtom(int atomIndex, std::ostream& stream)
{
	QVector<DataChannel*>::const_iterator channel = channels.constBegin();
	QVector<size_t>::const_iterator vcomp = vectorComponents.constBegin();
	for(; channel != channels.constEnd(); ++channel, ++vcomp) {
		if(channel != channels.constBegin()) stream << ' ';
		if(*channel != NULL) {
			if((*channel)->type() == qMetaTypeId<int>())
				stream << (*channel)->getIntComponent(atomIndex, *vcomp);
			else if((*channel)->type() == qMetaTypeId<FloatType>())
				stream << (*channel)->getFloatComponent(atomIndex, *vcomp);
		}
		else {
			stream << (atomIndex+1);
		}
	}
}

/******************************************************************************
 * Stores the data channels values for one atom in the given buffer
 * according to the OutputColumnMapping.
 *****************************************************************************/
void DataRecordWriterHelper::writeAtom(int atomIndex, double* buffer)
{
	QVector<DataChannel*>::const_iterator channel = channels.constBegin();
	QVector<size_t>::const_iterator vcomp = vectorComponents.constBegin();
	for(; channel != channels.constEnd(); ++channel, ++vcomp, ++buffer) {
		if(*channel != NULL) {
			if((*channel)->type() == qMetaTypeId<int>())
				*buffer = (*channel)->getIntComponent(atomIndex, *vcomp);
			else if((*channel)->type() == qMetaTypeId<FloatType>())
				*buffer = (*channel)->getFloatComponent(atomIndex, *vcomp);
			else *buffer = 0;
		}
		else {
			*buffer = atomIndex+1;
		}
	}
}


};	// End of namespace
