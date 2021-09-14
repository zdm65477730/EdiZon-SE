


void swap(datafile_t file, datafile_t file2) {
    datafile_t tempfile;
    tempfile.data_Dump = file.data_Dump;
    tempfile.filename = file.filename;
    file.data_Dump = file2.data_Dump;
    file.filename = file2.filename;
    file2.data_Dump = tempfile.data_Dump;
    file2.filename = tempfile.filename;
}

void GuiCheats::sortchunk(datafile_t file, int i, int bufferSize) {
    u8 *buffer = new u8[bufferSize];
    file.data_Dump->getData(i, buffer, bufferSize);

    // sort them here
    u32 buffer_inc, data_inc;
    if (m_64bit_offset) {
        buffer_inc = sizeof(fromto_t);
        data_inc = sizeof(u64);
    } else {
        buffer_inc = sizeof(fromto32_t);
        data_inc = sizeof(u32);
    }
    if (m_64bit_offset) {
        u64 *to;
        to = (u64 *)(buffer);
        std::sort(to, to + bufferSize / data_inc);
    } else {
        u32 *to;
        to = (u32 *)(buffer);
        std::sort(to, to + bufferSize / data_inc);
    }

    file.data_Dump->putData(i, buffer, bufferSize);
	delete buffer;
};

void GuiCheats::mergechunk(datafile_t file, datafile_t file2, int sortedsize){
    file2.data_Dump->clear();
    int filesize = file.data_Dump->size();
	int bufferSize1 = MAX_BUFFER_SIZE;
	int bufferSize2 = MAX_BUFFER_SIZE;
    // merge them here
    for (int i = 0; i < filesize; i += sortedsize * 2) {
        int index1 = 0;
        int index2 = 0;

        if (bufferSize1 > filesize - i) {
            bufferSize1 = filesize - i;
			bufferSize2 = 0;
        };
        if (bufferSize2 > filesize - i - sortedsize ) {
            bufferSize2 = filesize - i - sortedsize;
        };

        u8 *buffer1 = new u8[bufferSize1];
        file.data_Dump->getData(i, buffer1, bufferSize1);

        u8 *buffer2 = nullptr;
        if (bufferSize2 != 0) {
            buffer2 = new u8[bufferSize2];
            file.data_Dump->getData(i + sortedsize, buffer2, bufferSize2);
        }

        while (index1 < bufferSize1 && index2 < bufferSize2) {
            if (m_64bit_offset) {
                auto A = (u64 *)(buffer1 + index1);
                auto B = (u64 *)(buffer2 + index2);
                if (A < B) {
                    file2.data_Dump->addData((u8 *)&(A), 8);
                    index1 += 8;
                } else {
                    file2.data_Dump->addData((u8 *)&(B), 8);
                    index2 += 8;
                }
            } else {
                auto A = (u32 *)(buffer1 + index1);
                auto B = (u32 *)(buffer2 + index2);
                if (A < B) {
                    file2.data_Dump->addData((u8 *)&(A), 4);
                    index1 += 4;
                } else {
                    file2.data_Dump->addData((u8 *)&(B), 4);
                    index2 += 4;
                }
            }
        };
        if (index1 < bufferSize1) {
            file2.data_Dump->addData((u8 *)&(buffer1[index1]), bufferSize1 - index1);
        } else if (index2 < bufferSize2) {
            file2.data_Dump->addData((u8 *)&(buffer2[index2]), bufferSize2 - index2);
        }
    }

        file2.data_Dump->flushBuffer();
};

void GuiCheats::sortfile(datafile_t file, datafile_t file2) {
    if (file.data_Dump == nullptr) {
        file.data_Dump = new MemoryDump(file.filename.c_str(), DumpType::DATA, false);
    };
    if (file2.data_Dump == nullptr) {
        file2.data_Dump = new MemoryDump(file2.filename.c_str(), DumpType::DATA, false);
    };

    int filesize = file.data_Dump->size();
    int bufferSize = MAX_BUFFER_SIZE; 
    int sortedsize = bufferSize;
    // sort the chunks first
    for (int i = 0; i < filesize; i += bufferSize) {
        if (bufferSize > filesize - i) {
            bufferSize = filesize - i;
        };
        sortchunk(file, i, bufferSize);
    }
    // Merge sort the chunks
    while (sortedsize < filesize) {
        mergechunk(file, file2, sortedsize);
        swap(file, file2);
        sortedsize *= 2;
    };
}