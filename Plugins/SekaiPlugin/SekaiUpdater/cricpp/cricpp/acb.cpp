#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "utils.hpp"
#include "utf.hpp"
#include "afs2.hpp"
#include "acb.hpp"

namespace cricpp
{

ACBView ACBView::ParseACB(std::string_view buffer)
{
	ACBView view;
	view._utf = UTFView::ParseUTF(buffer);
	if (!view._utf.isParsed())
		throw std::runtime_error("Not ACB file");

	if (view._utf.GetPageCount() != 1)
		throw std::runtime_error("Unsupported utf page size: " 
			+ std::to_string(view._utf.GetPageCount()));

	view._AwsFile = view._utf.GetBytesView(0, "AwbFile");
	view._StreamAwbHash = view._utf.GetBytesView(0, "StreamAwbHash");

	std::string_view table_buffer = view._utf.GetBytesView(0, "WaveformTable");
	UTFView table_view = UTFView::ParseUTF(table_buffer);
	if (!table_view.isParsed())
		throw std::runtime_error("ACB does not contain WaveformTable");
	
	view._WaveformTable.resize(table_view.GetPageCount());
	for (size_t i = 0; i < table_view.GetPageCount(); i++)
	{
		view._WaveformTable[i].EncodeType = table_view.GetInt8Value(0, "EncodeType");
		view._WaveformTable[i].ExtensionData = table_view.GetInt16Value(0, "ExtensionData");
		view._WaveformTable[i].LoopFlag = table_view.GetInt8Value(0, "LoopFlag");
		view._WaveformTable[i].MemoryAwbId = table_view.GetInt16Value(0, "MemoryAwbId");
		view._WaveformTable[i].NumChannels = table_view.GetInt8Value(0, "NumChannels");
		view._WaveformTable[i].NumSamples = table_view.GetInt32Value(0, "NumSamples");
		view._WaveformTable[i].SamplingRate = table_view.GetInt16Value(0, "SamplingRate");
		view._WaveformTable[i].StreamAwbId = table_view.GetInt16Value(0, "StreamAwbId");
		view._WaveformTable[i].StreamAwbPortNo = table_view.GetInt16Value(0, "StreamAwbPortNo");
		view._WaveformTable[i].Streaming = table_view.GetInt8Value(0, "Streaming");

		if (view._WaveformTable[i].Streaming != 0)
			throw std::runtime_error("Streaming mode currently not supported");
	}

	view._afs2 = ASF2View::ParseASF2(view._AwsFile);
	if (!view._afs2.isParsed())
		throw std::runtime_error("Failed to parse ASF2");

	view._MemoryHcas.resize(view._afs2.GetFileCount());
	for (size_t i = 0; i < view._afs2.GetFileCount(); i++)
	{
		view._MemoryHcas[i] = view._afs2.GetFileView(i);
	}
	view._buffer = buffer;

	return view;
}

}