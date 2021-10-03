/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "install/http_nsp.hpp"

#include <switch.h>
#include <threads.h>
#include "data/buffered_placeholder_writer.hpp"
#include "util/title_util.hpp"
#include "util/error.hpp"
#include "util/debug.h"
#include "util/util.hpp"
#include "util/lang.hpp"
#include "ui/instPage.hpp"

namespace tin::install::nsp
{
    bool stopThreadsHttpNsp;

    HTTPNSP::HTTPNSP(std::string url) :
        m_download(url)
    {

    }

    struct StreamFuncArgs
    {
        tin::network::HTTPDownload* download;
        tin::data::BufferedPlaceholderWriter* bufferedPlaceholderWriter;
        u64 pfs0Offset;
        u64 ncaSize;
    };

    int CurlStreamFunc(void* in)
    {
        StreamFuncArgs* args = reinterpret_cast<StreamFuncArgs*>(in);

        auto streamFunc = [&](u8* streamBuf, size_t streamBufSize) -> size_t
        {
            while (true)
            {
                if (args->bufferedPlaceholderWriter->CanAppendData(streamBufSize))
                    break;
            }

            args->bufferedPlaceholderWriter->AppendData(streamBuf, streamBufSize);
            return streamBufSize;
        };

        if (args->download->StreamDataRange(args->pfs0Offset, args->ncaSize, streamFunc) == 1) stopThreadsHttpNsp = true;
        return 0;
    }

    int PlaceholderWriteFunc(void* in)
    {
        StreamFuncArgs* args = reinterpret_cast<StreamFuncArgs*>(in);

        while (!args->bufferedPlaceholderWriter->IsPlaceholderComplete() && !stopThreadsHttpNsp)
        {
            if (args->bufferedPlaceholderWriter->CanWriteSegmentToPlaceholder())
                args->bufferedPlaceholderWriter->WriteSegmentToPlaceholder();
        }

        return 0;
    }

    void HTTPNSP::StreamToPlaceholder(std::shared_ptr<nx::ncm::ContentStorage>& contentStorage, NcmContentId placeholderId)
    {
        const PFS0FileEntry* fileEntry = this->GetFileEntryByNcaId(placeholderId);
        std::string ncaFileName = this->GetFileEntryName(fileEntry);

        LOG_DEBUG("Retrieving %s\n", ncaFileName.c_str());
        size_t ncaSize = fileEntry->fileSize;

        tin::data::BufferedPlaceholderWriter bufferedPlaceholderWriter(contentStorage, placeholderId, ncaSize);
        StreamFuncArgs args;
        args.download = &m_download;
        args.bufferedPlaceholderWriter = &bufferedPlaceholderWriter;
        args.pfs0Offset = this->GetDataOffset() + fileEntry->dataOffset;
        args.ncaSize = ncaSize;
        thrd_t curlThread;
        thrd_t writeThread;

        stopThreadsHttpNsp = false;
        thrd_create(&curlThread, CurlStreamFunc, &args);
        thrd_create(&writeThread, PlaceholderWriteFunc, &args);

        u64 freq = armGetSystemTickFreq();
        u64 startTime = armGetSystemTick();
        size_t startSizeBuffered = 0;
        double speed = 0.0;

        inst::ui::instPage::setInstBarPerc(0);
        while (!bufferedPlaceholderWriter.IsBufferDataComplete() && !stopThreadsHttpNsp)
        {
            u64 newTime = armGetSystemTick();

            if (newTime - startTime >= freq * 0.5)
            {
                size_t newSizeBuffered = bufferedPlaceholderWriter.GetSizeBuffered();
                double mbBuffered = (newSizeBuffered / 1000000.0) - (startSizeBuffered / 1000000.0);
                double duration = ((double)(newTime - startTime) / (double)freq);
                speed =  mbBuffered / duration;

                startTime = newTime;
                startSizeBuffered = newSizeBuffered;

                int downloadProgress = (int)(((double)bufferedPlaceholderWriter.GetSizeBuffered() / (double)bufferedPlaceholderWriter.GetTotalDataSize()) * 100.0);

                inst::ui::instPage::setInstInfoText("inst.info_page.downloading"_lang + inst::util::formatUrlString(ncaFileName) + "inst.info_page.at"_lang + std::to_string(speed).substr(0, std::to_string(speed).size()-4) + "MB/s");
                inst::ui::instPage::setInstBarPerc((double)downloadProgress);
            }
        }
        inst::ui::instPage::setInstBarPerc(100);

        inst::ui::instPage::setInstInfoText("inst.info_page.top_info0"_lang + ncaFileName + "...");
        inst::ui::instPage::setInstBarPerc(0);
        while (!bufferedPlaceholderWriter.IsPlaceholderComplete() && !stopThreadsHttpNsp)
        {
            int installProgress = (int)(((double)bufferedPlaceholderWriter.GetSizeWrittenToPlaceholder() / (double)bufferedPlaceholderWriter.GetTotalDataSize()) * 100.0);

            inst::ui::instPage::setInstBarPerc((double)installProgress);
        }
        inst::ui::instPage::setInstBarPerc(100);

        thrd_join(curlThread, NULL);
        thrd_join(writeThread, NULL);
        if (stopThreadsHttpNsp) THROW_FORMAT(("inst.net.transfer_interput"_lang).c_str());
    }

    void HTTPNSP::BufferData(void* buf, off_t offset, size_t size)
    {
        m_download.BufferDataRange(buf, offset, size, nullptr);
    }
}