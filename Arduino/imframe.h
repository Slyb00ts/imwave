#define _frameSize  32

struct IMFrameHeader
{
	byte Function;
	byte SourceId;
	byte DestinationId;
	byte ReapeterId;
};

struct IMFrame
{
	IMFrameHeader Header;
	byte[_frameSize - sizeof(IMFrameHeader)] Body;
} imFrame;