//
// Created by 37496 on 2024/2/28.
//

#include "FileUtil.h"


namespace Tiny_muduo
{
    SmallFileUtil::SmallFileUtil(const char *file_name)
                :fd_(::open(file_name, O_RDONLY | O_CLOEXEC)),
                err_(0)
    {
        ::bzero(buf_, sizeof buf_);
        buf_[0] = '\0';
        if(fd_ < 0) {
            err_ = errno;
        }
    }

    SmallFileUtil::~SmallFileUtil() {
        if(fd_ >= 0) {
            ::close(fd_);
        }
    }

    int SmallFileUtil::readToString(int maxSize, std::string &content, int64_t *file_size, int64_t *modifyTime,
                                    int64_t *createTime) {
        int err = err_;
        if(fd_ >= 0) {
            content.clear();

            if(file_size) {
                struct stat statBuf;
                if(::fstat(fd_, &statBuf) == 0) {
                    if(S_ISREG(statBuf.st_mode)) {
                        *file_size = statBuf.st_size;
                        content.reserve(static_cast<int>(std::min(static_cast<int64_t>(maxSize), *file_size)));
                    }
                    else if(S_ISDIR(statBuf.st_mode)) {
                        err = EISDIR;
                    }

                    if(modifyTime) {
                        *modifyTime = statBuf.st_mtime;
                    }
                    if(createTime) {
                        *createTime = statBuf.st_ctime;
                    }
                }
                else {
                    err = errno;
                }
            }

            while (content.size() < static_cast<size_t>(maxSize)) {
                size_t toRead = std::min(static_cast<size_t>(maxSize)-content.size(), sizeof(buf_));
                ssize_t n = ::read(fd_, buf_, toRead);
                if (n > 0)
                {
                    content.append(buf_, n);
                }
                else
                {
                    if (n < 0)
                    {
                        err = errno;
                    }
                    break;
                }
            }

        }
        return err;
    }

    int SmallFileUtil::readToBuffer(int* size)
    {
        int err = err_;
        if (fd_ >= 0)
        {
            ssize_t n = ::pread(fd_, buf_, sizeof(buf_)-1, 0);
            if (n >= 0)
            {
                if (size)
                {
                    *size = static_cast<int>(n);
                }
                buf_[n] = '\0';
            }
            else
            {
                err = errno;
            }
        }
        return err;
    }

    int readSmallFile(const char* filename,
                      int maxSize,
                      std::string &content,
                      int64_t* fileSize,
                      int64_t* modifyTime,
                      int64_t* createTime)
    {
        SmallFileUtil file(filename);
        return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
    }

    File::File(const char* file_path)
            :filePath_(file_path)
            ,fd_(-1)
            ,size_(std::filesystem::file_size(file_path))
            ,isOpen_(false)
            ,isClose_(true){
    }


    File::STATE File::open(OPEN_MOD mod){
        if(isOpen_){
            return STATE::SUCCESS;
        }
        switch (mod)
        {
            case READ_ONLY:
                fd_ = ::open(filePath_.c_str(), O_RDONLY);
                break;
            case WRITE_ONLY:
                fd_ = ::open(filePath_.c_str(), O_WRONLY);
            case WRITE_APPEND:
                fd_ = ::open(filePath_.c_str(), O_WRONLY | O_APPEND);
            default:
                return STATE::FAIL;
                break;
        }
        if(fd_ == -1){
            return STATE::FAIL;
        }
        isOpen_ = true;
        isClose_ = false;
        return STATE::SUCCESS;
    }

    void File::close(){
        if(isOpen_){
            ::close(fd_);
        }
        isOpen_ = false;
        isClose_ = true;
    }


    FileSendStream::FileSendStream(const char* file_path, int outfd, bool isBlock, SEND_MOD mod)
            :file_(std::make_shared<File>(file_path))
            ,outfd_(outfd)
            ,isBlock_(isBlock)
            ,mod_(mod)
            ,mmap_(nullptr)
            ,sendFile_(nullptr)
            ,sended_bytes_(0){

        file_->open(File::OPEN_MOD::READ_ONLY);

        if(mod == MMAP){
            mmap_ = std::make_shared<MMap>(file_->getFd());
            remain_bytes_ = mmap_->getSize();
        }
        else{
            sendFile_ = std::make_shared<SendFile>(file_->getFd(), outfd_);
            remain_bytes_ = sendFile_->getSize();
        }
    }

    int FileSendStream::send(int *lastLen){
        int n = 0;
        int ret;
        if(mmap_){
            do{
                auto addr = mmap_->getAddress() + sended_bytes_;
                ret =  write(outfd_, addr, remain_bytes_);
                if(ret <= 0){
                    break;
                }
                n += ret;
                remain_bytes_ -= ret;
                sended_bytes_ += ret;
            }while(sendable() && !isBlock_);
        }
        else{
            do{
                ret = sendFile_->send(remain_bytes_);
                if(ret <= 0){
                    break;
                }
                n += ret;
                remain_bytes_ -= ret;
                sended_bytes_ += ret;
            }while(sendable() && !isBlock_);
        }

        if(lastLen){
            *lastLen = ret;
        }
        return n;
    }

}