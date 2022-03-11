void GuiCheats::searchMemoryAddressesPrimary32(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, searchRegion_t searchRegion, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos)
{
  (*displayDump) = new MemoryDump(EDIZON_DIR "/memdump1.dat", DumpType::ADDR, true);
  (*displayDump)->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  m_use_range = (searchMode == SEARCH_MODE_RANGE);
  (*displayDump)->setSearchParams(searchType, searchMode, searchRegion, searchValue1, searchValue2, m_use_range);

  MemoryDump *helperDump = new MemoryDump(EDIZON_DIR "/memdump1a.dat", DumpType::HELPER, true); // has address, size, count for fetching buffer from memory
  MemoryDump *newdataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, true);
  MemoryDump *newstringDump = new MemoryDump(EDIZON_DIR "/stringdump.csv", DumpType::DATA, true); // to del when not needed

  helperinfo_t helperinfo;
  helperinfo.count = 0;

  bool ledOn = false;

  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time primary search", unixTime1);
  // printf("main %lx main end %lx heap %lx heap end %lx \n",m_mainBaseAddr, m_mainBaseAddr+m_mainSize, m_heapBaseAddr, m_heapBaseAddr+m_heapSize);
  printf("value1=%lx value2=%lx typesize=%d\n", searchValue1._u64, searchValue2._u64, dataTypeSizes[searchType]);
  for (MemoryInfo meminfo : memInfos)
  {

    if (searchRegion == SEARCH_REGION_HEAP && meminfo.type != MemType_Heap)
      continue;
    else if (searchRegion == SEARCH_REGION_MAIN &&
             (((meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable && meminfo.type != MemType_CodeStatic) || !(meminfo.addr < m_mainend && meminfo.addr >= m_mainBaseAddr))))
      continue;
    else if (searchRegion == SEARCH_REGION_HEAP_AND_MAIN &&
             (((meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable)) || !((meminfo.addr < m_heapEnd && meminfo.addr >= m_heapBaseAddr) || (meminfo.addr < m_mainend && meminfo.addr >= m_mainBaseAddr))))
      continue;
    else if ( searchRegion == SEARCH_REGION_RAM && (meminfo.perm & Perm_Rw) != Perm_Rw) //searchRegion == SEARCH_REGION_RAM &&
      continue;

    // printf("%s%p", "meminfo.addr, ", meminfo.addr);
    // printf("%s%p", ", meminfo.end, ", meminfo.addr + meminfo.size);
    // printf("%s%p", ", meminfo.size, ", meminfo.size);
    // printf("%s%lx", ", meminfo.type, ", meminfo.type);
    // printf("%s%lx", ", meminfo.attr, ", meminfo.attr);
    // printf("%s%lx", ", meminfo.perm, ", meminfo.perm);
    // printf("%s%lx", ", meminfo.device_refcount, ", meminfo.device_refcount);
    // printf("%s%lx\n", ", meminfo.ipc_refcount, ", meminfo.ipc_refcount);
    setLedState(true);
    ledOn = !ledOn;

    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE; // consider to increase from 10k to 1M (not a big problem)
    u8 *buffer = new u8[bufferSize + 32];// make buffer size bigger to accomodate string that is at the border.
    while (offset < meminfo.size)
    {

      if (meminfo.size - offset < bufferSize)
        bufferSize = meminfo.size - offset;
      if (m_searchMode == SEARCH_MODE_STRING && (meminfo.size - offset > bufferSize + 32)) {
          debugger->readMemory(buffer, bufferSize + 32, meminfo.addr + offset);
      } else
          debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);

      searchValue_t realValue = {0};
      searchValue_t nextValue = {0};
      u32 inc_i;
      if (searchMode == SEARCH_MODE_POINTER || searchMode == SEARCH_MODE_EQA)
          inc_i = 4;
      else if (searchType == SEARCH_TYPE_UNSIGNED_16BIT)
        inc_i = 1;
      else
        inc_i = dataTypeSizes[searchType];

      for (u32 i = 0; i < bufferSize; i += inc_i)
      {
        u64 address = meminfo.addr + offset + i;
        memset(&realValue, 0, 8);
        if (searchMode == SEARCH_MODE_POINTER && m_32bitmode)
          memcpy(&realValue, buffer + i, 4);
        else
          memcpy(&realValue, buffer + i, dataTypeSizes[searchType]);
        if (Config::getConfig()->use_bitmask) {
            realValue._u64 = Config::getConfig()->bitmask & realValue._u64;
        }
        if (Config::getConfig()->exclude_ptr_candidates && searchMode != SEARCH_MODE_POINTER)
        {
          searchValue_t ptr_address;
          memcpy(&ptr_address, buffer + i - i % 8, 8);
          if (((ptr_address._u64 >= m_mainBaseAddr) && (ptr_address._u64 <= (m_mainend))) || ((ptr_address._u64 >= m_heapBaseAddr) && (ptr_address._u64 <= (m_heapEnd))))
            continue;
        }
        // if (_check_extra_not_OK(buffer, i)) continue; // if not match let's continue
        switch (searchMode)
        {
        case SEARCH_MODE_STRING: 
          {
            u32 pos = 0;
            while ((pos < strlen(m_searchString)) && (buffer[i + pos] == m_searchString[pos])) {
                pos++;
            };
            if (pos == strlen(m_searchString)) {
                (*displayDump)->addData((u8 *)&address, sizeof(u64));
                helperinfo.count++;
            };
          }; 
          break;
        case SEARCH_MODE_EQA:
          if (realValue._s32 == searchValue1._s32 || realValue._f32 == (float) searchValue1._s32 || realValue._f64 == (double) searchValue1._s32 )  {
                (*displayDump)->addData((u8 *)&address, sizeof(u64));
                helperinfo.count++;
            }
            break;
        case SEARCH_MODE_EQ:
          if (realValue._s32 == searchValue1._s32)
          {
            // if (Config::getConfig()->extra_value)
            // {
            //   if (!true) // extra value match
            //     break;
            // }
            (*displayDump)->addData((u8 *)&address, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_TWO_VALUES:
          if (realValue._s32 == searchValue1._s32) {
            memset(&nextValue, 0, 8);
            memcpy(&nextValue, buffer + i + dataTypeSizes[searchType], dataTypeSizes[searchType]);
            if (Config::getConfig()->use_bitmask) {
                nextValue._u64 = Config::getConfig()->bitmask & nextValue._u64;
            }
            if (nextValue._s32 == searchValue2._s32){
                (*displayDump)->addData((u8 *)&address, sizeof(u64));
                helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_TWO_VALUES_PLUS:  //BM need fixing
            if (realValue._s32 == searchValue1._s32) {
                s32 tvr = Config::getConfig()->two_value_range;
                for (s32 k = -tvr; k <= tvr; k++)
                    if ((k != 0) && (i + k * dataTypeSizes[searchType] >= 0) && (i + k * dataTypeSizes[searchType] + dataTypeSizes[searchType] <= bufferSize)) {
                        memset(&nextValue, 0, 8);
                        memcpy(&nextValue, buffer + i + k * dataTypeSizes[searchType], dataTypeSizes[searchType]);
                        if (Config::getConfig()->use_bitmask) {
                            nextValue._u64 = Config::getConfig()->bitmask & nextValue._u64;
                        }
                        if (nextValue._s32 == searchValue2._s32) {
                            (*displayDump)->addData((u8 *)&address, sizeof(u64));
                            helperinfo.count++;
                            break;
                        }
                    }
            }
            break;
        case SEARCH_MODE_NEQ:
          memset(&nextValue, 0, 8);
          memcpy(&nextValue, buffer + i + dataTypeSizes[searchType], dataTypeSizes[searchType]);
          if ((realValue._s32 xor nextValue._s32) == searchValue1._s32)
          {
            (*displayDump)->addData((u8 *)&address, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_GT:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s32 > searchValue1._s32)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 > searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_DIFFA:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s32 >= searchValue1._s32)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 >= searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_LT:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s32 < searchValue1._s32)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 < searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_SAMEA:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s32 <= searchValue1._s32)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 <= searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_RANGE:
          if (realValue._s32 >= searchValue1._s32 && realValue._s32 <= searchValue2._s32)
          {
            (*displayDump)->addData((u8 *)&address, sizeof(u64));
            newdataDump->addData((u8 *)&realValue, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_POINTER: //m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize
          if ((realValue._u64 != 0))
            if (((realValue._u64 >= m_mainBaseAddr) && (realValue._u64 <= (m_mainend))) || ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
            // if ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= m_heapEnd))
            {
              if ((m_forwarddump) && (address > realValue._u64) && (meminfo.type == MemType_Heap))
                break;
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&realValue, sizeof(u64));
              helperinfo.count++;
              // printf("%lx,%lx\n",address,realValue);
              // std::stringstream ss; // replace the printf
              // ss.str("");
              // ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << address;
              // ss << ",0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << realValue._u64;
              // char st[27];
              // snprintf(st, 27, "%s\n", ss.str().c_str());    //
              // newstringDump->addData((u8 *)&st, sizeof(st)); //
            }
          break;
        case SEARCH_MODE_NONE:
        case SEARCH_MODE_SAME:
        case SEARCH_MODE_DIFF:
        case SEARCH_MODE_NOTAB:
        case SEARCH_MODE_NOTA:
        case SEARCH_MODE_SAME_A:
        case SEARCH_MODE_SAME_B:
        case SEARCH_MODE_INC:
        case SEARCH_MODE_DEC:
        case SEARCH_MODE_INC_BY:
        case SEARCH_MODE_DEC_BY:
        case SEARCH_MODE_NOT_POINTER:
          printf("search mode non !");
          break;
        }
      }

      // helper info write must be before inc
      if (helperinfo.count != 0)
      {
        helperinfo.address = meminfo.addr + offset;
        helperinfo.size = bufferSize;
        helperDump->addData((u8 *)&helperinfo, sizeof(helperinfo));
        // printf("address 0x%lx ", helperinfo.address);
        // printf("size %ld ", helperinfo.size);
        // printf("count %ld type %d \n ", helperinfo.count, meminfo.type);
        helperinfo.count = 0;
      } // must be after write

      offset += bufferSize;
    }

    delete[] buffer;
  }

  setLedState(false);

  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);

  (*displayDump)->flushBuffer();
  newdataDump->flushBuffer();
  helperDump->flushBuffer();
  delete helperDump;
  delete newdataDump;
  newstringDump->flushBuffer(); // temp
  delete newstringDump;         //
}
//

void GuiCheats::searchMemoryAddressesSecondary32(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, bool use_range, MemoryDump **displayDump)
{
  MemoryDump *newDump = new MemoryDump(EDIZON_DIR "/memdump2.dat", DumpType::ADDR, true);
  bool ledOn = false;
  //begin
  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time Secondary search", unixTime1);
  if (searchMode == SEARCH_MODE_RANGE)
  {
    m_use_range = true;
    use_range = true;
  };
  u64 offset = 0;
  u64 bufferSize = MAX_BUFFER_SIZE; // this is for file access going for 1M
  u8 *buffer = new u8[bufferSize];
  // helper init
  MemoryDump *helperDump = new MemoryDump(EDIZON_DIR "/memdump1a.dat", DumpType::HELPER, false);   // has address, size, count for fetching buffer from memory
  MemoryDump *newhelperDump = new MemoryDump(EDIZON_DIR "/memdump3a.dat", DumpType::HELPER, true); // has address, size, count for fetching buffer from memory
  MemoryDump *newdataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, true);
  MemoryDump *debugdump1 = new MemoryDump(EDIZON_DIR "/debugdump1.dat", DumpType::HELPER, true);
  if (helperDump->size() == 0)
  {
    (new Snackbar("Helper file not found !"))->show();
    return;
  }
  else
  {
    // helper integrity check
    printf("start helper integrity check address secondary \n");
    u32 helpercount = 0;
    helperinfo_t helperinfo;
    for (u64 i = 0; i < helperDump->size(); i += sizeof(helperinfo))
    {
      helperDump->getData(i, &helperinfo, sizeof(helperinfo));
      helpercount += helperinfo.count;
    }
    if (helpercount != (*displayDump)->size() / sizeof(u64))
    {
      printf("Integrity problem with helper file helpercount = %d  memdumpsize = %ld \n", helpercount, (*displayDump)->size() / sizeof(u64));
      (new Snackbar("Helper integrity check failed !"))->show();
      return;
    }
    printf("end helper integrity check address secondary \n");
    // end helper integrity check

    std::stringstream Message;
    Message << "Traversing title memory.\n \nThis may take a while... secondary search\nTime " << (unixTime1 - time(NULL)) << "    total " << (*displayDump)->size();
    (new MessageBox(Message.str(), MessageBox::NONE))->show();
    requestDraw();
  }

  u8 *ram_buffer = new u8[bufferSize + 32];  // make buffer size bigger to accomodate string that is at the border.
  u64 helper_offset = 0;
  helperinfo_t helperinfo;
  helperinfo_t newhelperinfo;
  newhelperinfo.count = 0;

  helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo)); // helper_offset+=sizeof(helperinfo)
  if (m_searchMode == SEARCH_MODE_STRING && (debugger->queryMemory(helperinfo.address).size + debugger->queryMemory(helperinfo.address).addr < helperinfo.address + helperinfo.size + 32))
      debugger->readMemory(ram_buffer, helperinfo.size + 32, helperinfo.address);
  else
      debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
  // helper init end
  while (offset < (*displayDump)->size())
  {
    if ((*displayDump)->size() - offset < bufferSize)
      bufferSize = (*displayDump)->size() - offset;
    (*displayDump)->getData(offset, buffer, bufferSize); // BM4

    for (u64 i = 0; i < bufferSize; i += sizeof(u64)) // for (size_t i = 0; i < (bufferSize / sizeof(u64)); i++)
    {
      if (helperinfo.count == 0)
      {
        if (newhelperinfo.count != 0)
        {
          newhelperinfo.address = helperinfo.address;
          newhelperinfo.size = helperinfo.size;
          newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
          // printf("%s%lx\n", "newhelperinfo.address ", newhelperinfo.address);
          // printf("%s%lx\n", "newhelperinfo.size ", newhelperinfo.size);
          // printf("%s%lx\n", "newhelperinfo.count ", newhelperinfo.count);
          newhelperinfo.count = 0;
        }
        helper_offset += sizeof(helperinfo);
        helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo));
        debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
      }
      searchValue_t value = {0}, nextValue = {0};
      // searchValue_t testing = {0}; // temp
      u64 address = 0;

      address = *reinterpret_cast<u64 *>(&buffer[i]); //(*displayDump)->getData(i * sizeof(u64), &address, sizeof(u64));

      memcpy(&value, ram_buffer + address - helperinfo.address, dataTypeSizes[searchType]); // extrat from buffer instead of making call
      if (Config::getConfig()->use_bitmask) {
          value._u64 = Config::getConfig()->bitmask & value._u64;
      }
      helperinfo.count--;                                                                   // each fetch dec
      // testing = value;                                                                      // temp
      // debugger->readMemory(&value, dataTypeSizes[searchType], address);
      // if (testing._u64 != value._u64)
      // {
      //   printf("%s%lx\n", "helperinfo.address ", helperinfo.address);
      //   printf("%s%lx\n", "helperinfo.size ", helperinfo.size);
      //   printf("%s%lx\n", "helperinfo.count ", helperinfo.count);
      //   printf("%s%lx\n", "address ", address);
      //   printf("%s%lx\n", "testing._u64 ", testing._u64);
      //   printf("%s%lx\n", "value ", value);
      //   printf("%s%lx\n", " address - helperinfo.address ", address - helperinfo.address);
      //   printf("%s%lx\n", " * (ram_buffer + address - helperinfo.address) ", *(ram_buffer + address - helperinfo.address));
      //   printf("%s%lx\n", " * (&ram_buffer[ address - helperinfo.address]) ", *(&ram_buffer[address - helperinfo.address]));
      //   printf("%s%lx\n", "  (ram_buffer + address - helperinfo.address) ", (ram_buffer + address - helperinfo.address));
      //   printf("%s%lx\n", "  (&ram_buffer[ address - helperinfo.address]) ", (&ram_buffer[address - helperinfo.address]));
      //   printf("%s%lx\n", "  helperinfo.size - address + helperinfo.address ", helperinfo.size - address + helperinfo.address);
      //   // debugdump1->addData((u8 *)&ram_buffer, helperinfo.size);
      //   // debugger->readMemory(ram_buffer, 0x50, address);
      //   // debugdump2->addData((u8 *)&ram_buffer, 0x50);
      //   //
      //   // delete debugdump2;
      // }

      if (i % 50000 == 0)
      {
        setLedState(true);
        ledOn = !ledOn;
      }

      switch (searchMode)
      {
      case SEARCH_MODE_STRING: 
        {
          u32 pos = 0;
          while ((pos < strlen(m_searchString)) && (ram_buffer[address - helperinfo.address + pos] == m_searchString[pos])) {
              pos++;
          };
          if (pos == strlen(m_searchString)) {
              newDump->addData((u8 *)&address, sizeof(u64));
              newhelperinfo.count++;
          };
        }; 
        break;  
      case SEARCH_MODE_EQA:
        if (value._s32 == searchValue1._s32 || value._f32 == (float) searchValue1._s32 || value._f64 == (double) searchValue1._s32 )  
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_EQ:
        if (value._s32 == searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_TWO_VALUES:
        if (value._s32 == searchValue1._s32) {
          memset(&nextValue, 0, 8);
          memcpy(&nextValue, ram_buffer + address - helperinfo.address + dataTypeSizes[searchType], dataTypeSizes[searchType]);
          if (Config::getConfig()->use_bitmask) {
              nextValue._u64 = Config::getConfig()->bitmask & nextValue._u64;
          }
          if (nextValue._s32 == searchValue2._s32){
            newDump->addData((u8 *)&address, sizeof(u64));
            newhelperinfo.count++;
          }
        }
        break;
      case SEARCH_MODE_TWO_VALUES_PLUS:  //BM need fixing
          if (value._s32 == searchValue1._s32) {
              s32 tvr = Config::getConfig()->two_value_range;
              for (s32 k = -tvr; k <= tvr; k++)
                  if ((k != 0) && (i + k * dataTypeSizes[searchType] >= 0) && (i + k * dataTypeSizes[searchType] + dataTypeSizes[searchType] <= bufferSize)) {
                      // for (s32 k = -3; k <= 3; k++)
                      //     if (k != 0) {
                      memset(&nextValue, 0, 8);
                      memcpy(&nextValue, ram_buffer + address - helperinfo.address + k * dataTypeSizes[searchType], dataTypeSizes[searchType]);
                      if (Config::getConfig()->use_bitmask) {
                          nextValue._u64 = Config::getConfig()->bitmask & nextValue._u64;
                      }
                      if (nextValue._s32 == searchValue2._s32) {
                          newDump->addData((u8 *)&address, sizeof(u64));
                          newhelperinfo.count++;
                          break;
                      }
                  }
          }
          break;
      case SEARCH_MODE_NEQ:
        if (value._s32 != searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_GT:
        if (value._s32 > searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFFA:
        if (value._s32 >= searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_LT:
        if (value._s32 < searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_SAMEA:
        if (value._s32 <= searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_RANGE:
        if (value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64)); // add here
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_POINTER: //m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize
        if ((value._u64 >= m_mainBaseAddr && value._u64 <= (m_mainend)) || (value._u64 >= m_heapBaseAddr && value._u64 <= (m_heapEnd)))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_SAME:
        if (value._s32 == searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFF:
      case SEARCH_MODE_NOTAB:
      case SEARCH_MODE_NOTA:
        if (value._s32 != searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_INC:
        if (value._s32 > searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DEC:
        if (value._s32 < searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_INC_BY:
      case SEARCH_MODE_DEC_BY:
      case SEARCH_MODE_NONE:
      case SEARCH_MODE_SAME_A:
      case SEARCH_MODE_SAME_B:
      case SEARCH_MODE_NOT_POINTER:
        break;
      }
    }
    printf("%ld of %ld done \n", offset, (*displayDump)->size()); // maybe consider message box this info
    offset += bufferSize;
  }

  if (newhelperinfo.count != 0) // take care of the last one
  {
    newhelperinfo.address = helperinfo.address;
    newhelperinfo.size = helperinfo.size;
    newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
    // printf("%s%lx\n", "newhelperinfo.address ", newhelperinfo.address);
    // printf("%s%lx\n", "newhelperinfo.size ", newhelperinfo.size);
    // printf("%s%lx\n", "newhelperinfo.count ", newhelperinfo.count);
    newhelperinfo.count = 0;
  }
  //end
  newDump->flushBuffer();
  newhelperDump->flushBuffer();
  newdataDump->flushBuffer();

  if (newDump->size() > 0)
  {
    // delete m_memoryDump;
    // remove(EDIZON_DIR "/memdump1.dat");
    // rename(EDIZON_DIR "/memdump2.dat", EDIZON_DIR "/memdump2.dat");
    (*displayDump)->clear();
    (*displayDump)->setSearchParams(searchType, searchMode, (*displayDump)->getDumpInfo().searchRegion, searchValue1, searchValue2, use_range);
    (*displayDump)->setDumpType(DumpType::ADDR);

    // begin copy
    offset = 0;
    bufferSize = MAX_BUFFER_SIZE;                 //0x1000000; // match what was created before
    printf("%s%lx\n", "bufferSize ", bufferSize); // printf
    while (offset < newDump->size())
    {
      if (newDump->size() - offset < bufferSize)
        bufferSize = newDump->size() - offset;
      newDump->getData(offset, buffer, bufferSize);
      (*displayDump)->addData(buffer, bufferSize);
      offset += bufferSize;
    }
    // end copy

    (*displayDump)->flushBuffer();
  }
  else
  {
    (new Snackbar("None of values changed to the entered one!"))->show();
    m_nothingchanged = true;
  }

  setLedState(false);
  delete newDump;
  delete newhelperDump;
  delete helperDump;
  delete debugdump1;
  delete newdataDump;
  delete[] buffer;
  delete[] ram_buffer;

  remove(EDIZON_DIR "/memdump2.dat");
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time secondary search ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
}

void GuiCheats::searchMemoryAddressesSecondary232(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, MemoryDump **displayDump)
{
  MemoryDump *lastdataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, false);
  if (lastdataDump->size() == 0)
  {
    (new Snackbar("No previous value found !"))->show();
    m_nothingchanged = true;
    delete lastdataDump;
    return;
  }
  else
  {
    delete lastdataDump;
  }
  MemoryDump *newDump = new MemoryDump(EDIZON_DIR "/memdump2.dat", DumpType::ADDR, true);
  bool ledOn = false;
  //begin
  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time Secondary search", unixTime1);
  if (searchMode == SEARCH_MODE_RANGE)
    m_use_range = true;
  u64 offset = 0;
  u64 bufferSize = MAX_BUFFER_SIZE; // this is for file access going for 1M
  u8 *buffer = new u8[bufferSize];
  u8 *predatabuffer = new u8[bufferSize];
  u8 *predatabufferB = new u8[bufferSize];
  searchValue_t prevalue = {0};
  searchValue_t valueB = {0};
  // helper init
  MemoryDump *helperDump = new MemoryDump(EDIZON_DIR "/memdump1a.dat", DumpType::HELPER, false);   // has address, size, count for fetching buffer from memory
  MemoryDump *newhelperDump = new MemoryDump(EDIZON_DIR "/memdump3a.dat", DumpType::HELPER, true); // has address, size, count for fetching buffer from memory
  std::string s = m_edizon_dir + "/datadump2.dat" + m_store_extension;
  REPLACEFILE(s.c_str(), EDIZON_DIR "/predatadump2.dat");
  MemoryDump *predataDump = new MemoryDump(EDIZON_DIR "/predatadump2.dat", DumpType::DATA, false);
  MemoryDump *newdataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, true);
  s = m_edizon_dir + "/datadump2B.dat" + m_store_extension;
  if (access(s.c_str(), F_OK) == 0) {
      REPLACEFILE(s.c_str(), EDIZON_DIR "/predatadump2B.dat");
  }
  MemoryDump *predataDumpB = new MemoryDump(EDIZON_DIR "/predatadump2B.dat", DumpType::DATA, false);
  MemoryDump *newdataDumpB = new MemoryDump(EDIZON_DIR "/datadump2B.dat", DumpType::DATA, true);
  // MemoryDump *debugdump1 = new MemoryDump(EDIZON_DIR "/debugdump1.dat", DumpType::HELPER, true);
  if (helperDump->size() == 0)
  {
    (new Snackbar("Helper file not found !"))->show();
    return;
  }
  else
  {
    // helper integrity check
    printf("start helper integrity check address secondary \n");
    u32 helpercount = 0;
    helperinfo_t helperinfo;
    for (u64 i = 0; i < helperDump->size(); i += sizeof(helperinfo))
    {
      helperDump->getData(i, &helperinfo, sizeof(helperinfo));
      helpercount += helperinfo.count;
    }
    if (helpercount != (*displayDump)->size() / sizeof(u64))
    {
      printf("Integrity problem with helper file helpercount = %d  memdumpsize = %ld \n", helpercount, (*displayDump)->size() / sizeof(u64));
      (new Snackbar("Helper integrity check failed !"))->show();
      return;
    }
    printf("end helper integrity check address secondary \n");
    // end helper integrity check

    std::stringstream Message;
    Message << "Traversing title memory.\n \nThis may take a while... secondary search\nTime " << (unixTime1 - time(NULL)) << "    total " << (*displayDump)->size();
    (new MessageBox(Message.str(), MessageBox::NONE))->show();
    requestDraw();
  }

  u8 *ram_buffer = new u8[bufferSize];
  u64 helper_offset = 0;
  helperinfo_t helperinfo;
  helperinfo_t newhelperinfo;
  newhelperinfo.count = 0;

  helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo)); // helper_offset+=sizeof(helperinfo)
  debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
  // helper init end
  while (offset < (*displayDump)->size())
  {
    if ((*displayDump)->size() - offset < bufferSize)
      bufferSize = (*displayDump)->size() - offset;
    (*displayDump)->getData(offset, buffer, bufferSize); // BM4
    predataDump->getData(offset, predatabuffer, bufferSize);
    predataDumpB->getData(offset, predatabufferB, bufferSize);

    for (u64 i = 0; i < bufferSize; i += sizeof(u64)) // for (size_t i = 0; i < (bufferSize / sizeof(u64)); i++)
    {
      if (helperinfo.count == 0)
      {
        if (newhelperinfo.count != 0)
        {
          newhelperinfo.address = helperinfo.address;
          newhelperinfo.size = helperinfo.size;
          newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
          // printf("%s%lx\n", "newhelperinfo.address ", newhelperinfo.address);
          // printf("%s%lx\n", "newhelperinfo.size ", newhelperinfo.size);
          // printf("%s%lx\n", "newhelperinfo.count ", newhelperinfo.count);
          newhelperinfo.count = 0;
        }
        helper_offset += sizeof(helperinfo);
        helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo));
        debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
      }
      searchValue_t value = {0}, nextValue = {0};
      // searchValue_t testing = {0}; // temp
      u64 address = 0;

      address = *reinterpret_cast<u64 *>(&buffer[i]); //(*displayDump)->getData(i * sizeof(u64), &address, sizeof(u64));
      prevalue._u64 = *reinterpret_cast<u64 *>(&predatabuffer[i]);
      valueB._u64 = *reinterpret_cast<u64 *>(&predatabufferB[i]);

      memcpy(&value, ram_buffer + address - helperinfo.address, dataTypeSizes[searchType]); // extrat from buffer instead of making call
      helperinfo.count--;                                                                   // each fetch dec
      // testing = value;                                                                      // temp
      // debugger->readMemory(&value, dataTypeSizes[searchType], address);
      // if (testing._u64 != value._u64)
      // {
      //   printf("%s%lx\n", "helperinfo.address ", helperinfo.address);
      //   printf("%s%lx\n", "helperinfo.size ", helperinfo.size);
      //   printf("%s%lx\n", "helperinfo.count ", helperinfo.count);
      //   printf("%s%lx\n", "address ", address);
      //   printf("%s%lx\n", "testing._u64 ", testing._u64);
      //   printf("%s%lx\n", "value ", value);
      //   printf("%s%lx\n", " address - helperinfo.address ", address - helperinfo.address);
      //   printf("%s%lx\n", " * (ram_buffer + address - helperinfo.address) ", *(ram_buffer + address - helperinfo.address));
      //   printf("%s%lx\n", " * (&ram_buffer[ address - helperinfo.address]) ", *(&ram_buffer[address - helperinfo.address]));
      //   printf("%s%lx\n", "  (ram_buffer + address - helperinfo.address) ", (ram_buffer + address - helperinfo.address));
      //   printf("%s%lx\n", "  (&ram_buffer[ address - helperinfo.address]) ", (&ram_buffer[address - helperinfo.address]));
      //   printf("%s%lx\n", "  helperinfo.size - address + helperinfo.address ", helperinfo.size - address + helperinfo.address);
      //   // debugdump1->addData((u8 *)&ram_buffer, helperinfo.size);
      //   // debugger->readMemory(ram_buffer, 0x50, address);
      //   // debugdump2->addData((u8 *)&ram_buffer, 0x50);
      //   //
      //   // delete debugdump2;
      // }

      if (i % 50000 == 0)
      {
        setLedState(true);
        ledOn = !ledOn;
      }

      switch (searchMode)
      {
      case SEARCH_MODE_EQ:
        if (value._s32 == searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_TWO_VALUES:
        if (value._s32 == searchValue1._s32) {
          memset(&nextValue, 0, 8);
          memcpy(&nextValue, ram_buffer + address - helperinfo.address + dataTypeSizes[searchType], dataTypeSizes[searchType]);
          if (nextValue._s32 == searchValue2._s32){
            newDump->addData((u8 *)&address, sizeof(u64));
            newhelperinfo.count++;
          }
        }
        break;
      case SEARCH_MODE_TWO_VALUES_PLUS: //BM need fixing
          if (value._s32 == searchValue1._s32) {
              // for (s32 k = -3; k <= 3; k++)
              //     if (k != 0) {
              s32 tvr = Config::getConfig()->two_value_range;
              for (s32 k = -tvr; k <= tvr; k++)
                  if ((k != 0) && (i + k * dataTypeSizes[searchType] >= 0) && (i + k * dataTypeSizes[searchType] + dataTypeSizes[searchType] <= bufferSize)) {
                      memset(&nextValue, 0, 8);
                      memcpy(&nextValue, ram_buffer + address - helperinfo.address + k * dataTypeSizes[searchType], dataTypeSizes[searchType]);
                      if (nextValue._s32 == searchValue2._s32) {
                          newDump->addData((u8 *)&address, sizeof(u64));
                          newhelperinfo.count++;
                          break;
                      }
                  }
          }
        break;
      case SEARCH_MODE_NEQ:
        if (value._s32 != searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_GT:
        if (value._s32 > searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFFA: //need to rewrite
        if (value._s32 != prevalue._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_LT:
        if (value._s32 < searchValue1._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_SAMEA: // need to rewrite
        if (value._s32 == prevalue._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_RANGE:
        if (value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32)
        {
          newDump->addData((u8 *)&address, sizeof(u64)); // add here
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_POINTER: //m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize
        if ((value._u64 >= m_mainBaseAddr && value._u64 <= (m_mainend)) || (value._u64 >= m_heapBaseAddr && value._u64 <= (m_heapEnd)))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_SAME:
        if ((value._s32 == prevalue._s32) && ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFF:
        if ((value._s32 != prevalue._s32) && ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_NOTAB:
          if ((value._s32 != prevalue._s32) && (value._s32 != valueB._s32) && ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range)) {
              newDump->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&prevalue, sizeof(u64));
              newdataDumpB->addData((u8 *)&valueB, sizeof(u64));
              newhelperinfo.count++;
          }
          break;
      case SEARCH_MODE_DIFFBA:
          if (value._f32 == (valueB._f32 * 2 - prevalue._f32) && (value._f32 - valueB._f32) != 0) {
              newDump->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&valueB, sizeof(u64));
              newdataDumpB->addData((u8 *)&value, sizeof(u64));
              newhelperinfo.count++;
          }
          break;
      case SEARCH_MODE_NOTA:
          if ((value._s32 != prevalue._s32) && ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range)) {
              newDump->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&prevalue, sizeof(u64));
              newdataDumpB->addData((u8 *)&value, sizeof(u64));
              newhelperinfo.count++;
          }
          break;
      case SEARCH_MODE_SAME_A:
          if ((value._s32 == prevalue._s32) && ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range)) {
              newDump->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&prevalue, sizeof(u64));
              newdataDumpB->addData((u8 *)&valueB, sizeof(u64));
              newhelperinfo.count++;
          }
          break;
      case SEARCH_MODE_SAME_B:
          if ((value._s32 == valueB._s32) && ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range)) {
              newDump->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&prevalue, sizeof(u64));
              newdataDumpB->addData((u8 *)&valueB, sizeof(u64));
              newhelperinfo.count++;
          }
          break;
      case SEARCH_MODE_INC:
        if ((value._s32 > prevalue._s32)&& ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DEC:
        if ((value._s32 < prevalue._s32)&& ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_INC_BY:
          if ((value._s32 == prevalue._s32 + searchValue1._s32) && ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range)) {
              newDump->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&value, sizeof(u64));
              newhelperinfo.count++;
          }
          break;
      case SEARCH_MODE_DEC_BY:
          if ((value._s32 == prevalue._s32 - searchValue1._s32) && ((value._s32 >= searchValue1._s32 && value._s32 <= searchValue2._s32) || !m_use_range)) {
              newDump->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&value, sizeof(u64));
              newhelperinfo.count++;
          }
          break;
      case SEARCH_MODE_NONE:
      case SEARCH_MODE_NOT_POINTER:
        break;
      }
    }
    printf("%ld of %ld done \n", offset, (*displayDump)->size()); // maybe consider message box this info
    offset += bufferSize;
  }

  if (newhelperinfo.count != 0) // take care of the last one
  {
    newhelperinfo.address = helperinfo.address;
    newhelperinfo.size = helperinfo.size;
    newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
    // printf("%s%lx\n", "newhelperinfo.address ", newhelperinfo.address);
    // printf("%s%lx\n", "newhelperinfo.size ", newhelperinfo.size);
    // printf("%s%lx\n", "newhelperinfo.count ", newhelperinfo.count);
    newhelperinfo.count = 0;
  }
  //end
  newDump->flushBuffer();
  newhelperDump->flushBuffer();
  newdataDump->flushBuffer();
  newdataDumpB->flushBuffer();

  if (newDump->size() > 0)
  {
    // delete m_memoryDump;
    // remove(EDIZON_DIR "/memdump1.dat");
    // rename(EDIZON_DIR "/memdump2.dat", EDIZON_DIR "/memdump2.dat");
    (*displayDump)->clear();
    (*displayDump)->setSearchParams(searchType, searchMode, (*displayDump)->getDumpInfo().searchRegion, searchValue1, searchValue2, m_use_range);
    (*displayDump)->setDumpType(DumpType::ADDR);

    // begin copy
    offset = 0;
    bufferSize = MAX_BUFFER_SIZE;                 //0x1000000; // match what was created before
    printf("%s%lx\n", "bufferSize ", bufferSize); // printf
    while (offset < newDump->size())
    {
      if (newDump->size() - offset < bufferSize)
        bufferSize = newDump->size() - offset;
      newDump->getData(offset, buffer, bufferSize);
      (*displayDump)->addData(buffer, bufferSize);
      offset += bufferSize;
    }
    // end copy

    (*displayDump)->flushBuffer();
  }
  else
  {
    (new Snackbar("None of values changed to the entered one!"))->show();
    m_nothingchanged = true;
  }

  setLedState(false);
  delete newDump;
  delete newhelperDump;
  delete helperDump;
  // delete debugdump1;
  delete newdataDump;
  delete newdataDumpB;
  delete predataDump;
  delete predataDumpB;
  delete[] buffer;
  delete[] predatabuffer;
  delete[] predatabufferB;
  delete[] ram_buffer;

  remove(EDIZON_DIR "/memdump2.dat");
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time secondary search ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
}
