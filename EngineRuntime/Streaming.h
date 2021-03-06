#pragma once

#include "EngineBase.h"
#include "PlatformDependent/FileApi.h"

namespace Engine
{
	namespace Streaming
	{
		enum FileAccess { AccessRead, AccessWrite, AccessReadWrite };
		enum FileCreationMode { CreateAlways, CreateNew, OpenAlways, OpenExisting, TruncateExisting };
		enum SeekOrigin { Begin, Current, End };

		class Stream : public Object
		{
		public:
			virtual void Read(void * buffer, uint32 length) = 0;
			virtual void Write(const void * data, uint32 length) = 0;
			virtual int64 Seek(int64 position, SeekOrigin origin) = 0;
			virtual uint64 Length(void) = 0;
			virtual void SetLength(uint64 length) = 0;
			virtual void Flush(void) = 0;
			void CopyTo(Stream * to, uint64 length);
			void CopyTo(Stream * to);
		};

		class FileStream final : public Stream
		{
			handle file;
			bool owned;
		public:
			FileStream(const string & path, FileAccess access, FileCreationMode mode);
			FileStream(handle file_handle, bool take_control = false);
			virtual void Read(void * buffer, uint32 length) override;
			virtual void Write(const void * data, uint32 length) override;
			virtual int64 Seek(int64 position, SeekOrigin origin) override;
			virtual uint64 Length(void) override;
			virtual void SetLength(uint64 length) override;
			virtual void Flush(void) override;
			virtual ~FileStream(void) override;
			virtual string ToString(void) const override;

			handle Handle(void) const;
			handle & Handle(void);
		};
		class MemoryStream final : public Stream
		{
			Array<uint8> data;
			int32 pointer;
		public:
			MemoryStream(const void * source, int length);
			MemoryStream(const void * source, int length, int block);
			MemoryStream(Stream * source, int length);
			MemoryStream(Stream * source, int length, int block);
			MemoryStream(int block);
			virtual void Read(void * buffer, uint32 length) override;
			virtual void Write(const void * data, uint32 length) override;
			virtual int64 Seek(int64 position, SeekOrigin origin) override;
			virtual uint64 Length(void) override;
			virtual void SetLength(uint64 length) override;
			virtual void Flush(void) override;
			virtual ~MemoryStream(void) override;
			virtual string ToString(void) const override;

			void * GetBuffer(void);
			const void * GetBuffer(void) const;
		};
		class FragmentStream final : public Stream
		{
			Stream * inner;
			uint64 begin, end;
			int64 pointer;
		public:
			FragmentStream(Stream * Inner, uint64 From, uint64 Length);			
			virtual void Read(void * buffer, uint32 length) override;
			virtual void Write(const void * data, uint32 length) override;
			virtual int64 Seek(int64 position, SeekOrigin origin) override;
			virtual uint64 Length(void) override;
			virtual void SetLength(uint64 length) override;
			virtual void Flush(void) override;
			virtual ~FragmentStream(void) override;
			virtual string ToString(void) const override;
		};

		class TextWriter final : public Object
		{
			Stream * dest;
			Encoding coding;
		public:
			TextWriter(Stream * Dest);
			TextWriter(Stream * Dest, Encoding encoding);
			void Write(const string & text) const;
			void WriteLine(const string & text) const;
			void WriteEncodingSignature(void) const;
			virtual ~TextWriter(void) override;
			virtual string ToString(void) const override;

			TextWriter & operator << (const string & text);
			const TextWriter & operator << (const string & text) const;
		};
		class TextReader final : public Object
		{
			Stream * source;
			mutable Encoding coding;
			mutable bool eof;
		public:
			TextReader(Stream * Source);
			TextReader(Stream * Source, Encoding encoding);
			uint32 ReadChar(void) const;
			string ReadLine(void) const;
			string ReadAll(void) const;
			bool EofReached(void) const;
			Encoding GetEncoding(void) const;
			virtual ~TextReader(void) override;
			virtual string ToString(void) const override;

			TextReader & operator >> (string & str);
			const TextReader & operator >> (string & str) const;
		};
	}
}