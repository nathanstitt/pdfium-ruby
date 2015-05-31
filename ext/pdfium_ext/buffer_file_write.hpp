// Implementation of FPDF_FILEWRITE into a file.
class BufferFileWrite : public FPDF_FILEWRITE {
public:
    BufferFileWrite( const std::string &file ) :
        _file( file, std::ios::out | std::ios::binary )
    {
        version = 1;
        WriteBlock = &WriteBlockImpl;
    }
    ~BufferFileWrite() {
        _file.close();
    }

private:
    int DoWriteBlock(const void* data, unsigned long size){
        _file.write(static_cast<const char*>(data), size);
        return 1;
    }
    static int WriteBlockImpl(FPDF_FILEWRITE* this_file_write, const void* data,
                              unsigned long size){
        BufferFileWrite* mem_buffer_file_write =
            static_cast<BufferFileWrite*>(this_file_write);
        return mem_buffer_file_write->DoWriteBlock(data, size);
    }

    std::ofstream _file;
};
