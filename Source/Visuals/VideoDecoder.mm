// Import macOS frameworks BEFORE JuceHeader to avoid Component name clash
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>

#include "VideoDecoder.h"

struct VideoDecoder::Impl
{
    AVAsset* asset = nil;
    AVAssetImageGenerator* imageGenerator = nil;
    double duration = 0.0;
    float fps = 30.0f;
    int totalFrames = 0;
    juce::String currentPath;

    // Frame cache: frame index -> decoded image
    std::map<int, juce::Image> frameCache;
    std::vector<int> cacheOrder;
    static constexpr int maxCacheSize = 64;

    void clearCache()
    {
        frameCache.clear();
        cacheOrder.clear();
    }

    void addToCache (int frameIndex, const juce::Image& image)
    {
        if (frameCache.count (frameIndex) > 0)
            return;

        if ((int) frameCache.size() >= maxCacheSize)
        {
            int oldest = cacheOrder.front();
            cacheOrder.erase (cacheOrder.begin());
            frameCache.erase (oldest);
        }

        frameCache[frameIndex] = image;
        cacheOrder.push_back (frameIndex);
    }

    juce::Image* getCached (int frameIndex)
    {
        auto it = frameCache.find (frameIndex);
        if (it != frameCache.end())
            return &it->second;
        return nullptr;
    }
};

VideoDecoder::VideoDecoder() : pimpl (std::make_unique<Impl>()) {}

VideoDecoder::~VideoDecoder()
{
    if (pimpl->imageGenerator != nil)
        pimpl->imageGenerator = nil;
    if (pimpl->asset != nil)
        pimpl->asset = nil;
}

bool VideoDecoder::openFile (const juce::String& path)
{
    @autoreleasepool
    {
        if (path == pimpl->currentPath && pimpl->asset != nil)
            return true;

        pimpl->clearCache();
        pimpl->asset = nil;
        pimpl->imageGenerator = nil;
        pimpl->currentPath = path;

        NSURL* url = [NSURL fileURLWithPath: [NSString stringWithUTF8String: path.toRawUTF8()]];
        AVAsset* asset = [AVAsset assetWithURL: url];

        if (asset == nil)
            return false;

        // Check the asset has video tracks
        NSArray<AVAssetTrack*>* videoTracks = [asset tracksWithMediaType: AVMediaTypeVideo];
        if ([videoTracks count] == 0)
            return false;

        AVAssetTrack* videoTrack = [videoTracks firstObject];
        pimpl->fps = videoTrack.nominalFrameRate;
        if (pimpl->fps <= 0.0f)
            pimpl->fps = 30.0f;

        pimpl->duration = CMTimeGetSeconds (asset.duration);
        if (pimpl->duration <= 0.0)
            return false;

        pimpl->totalFrames = (int) (pimpl->duration * pimpl->fps);
        if (pimpl->totalFrames <= 0)
            pimpl->totalFrames = 1;

        pimpl->asset = asset;

        AVAssetImageGenerator* generator = [[AVAssetImageGenerator alloc] initWithAsset: asset];
        generator.appliesPreferredTrackTransform = YES;
        generator.requestedTimeToleranceBefore = kCMTimeZero;
        generator.requestedTimeToleranceAfter = kCMTimeZero;

        pimpl->imageGenerator = generator;
        return true;
    }
}

bool VideoDecoder::isOpen() const
{
    return pimpl->asset != nil && pimpl->imageGenerator != nil;
}

double VideoDecoder::getDurationSeconds() const
{
    return pimpl->duration;
}

juce::Image VideoDecoder::getFrameAtPhase (float phase)
{
    if (! isOpen())
        return {};

    @autoreleasepool
    {
        phase = std::fmod (phase, 1.0f);
        if (phase < 0.0f)
            phase += 1.0f;

        int frameIndex = (int) (phase * pimpl->totalFrames);
        frameIndex = juce::jlimit (0, pimpl->totalFrames - 1, frameIndex);

        // Check cache
        if (auto* cached = pimpl->getCached (frameIndex))
            return *cached;

        // Decode frame
        double timeSeconds = (double) frameIndex / pimpl->fps;
        CMTime time = CMTimeMakeWithSeconds (timeSeconds, 600);

        NSError* error = nil;
        CGImageRef cgImage = [pimpl->imageGenerator copyCGImageAtTime: time
                                                           actualTime: nullptr
                                                                error: &error];

        if (cgImage == nullptr)
            return {};

        int w = (int) CGImageGetWidth (cgImage);
        int h = (int) CGImageGetHeight (cgImage);

        juce::Image juceImage (juce::Image::ARGB, w, h, true);

        {
            juce::Image::BitmapData bitmapData (juceImage, juce::Image::BitmapData::writeOnly);

            CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
            CGContextRef context = CGBitmapContextCreate (
                bitmapData.data,
                (size_t) w, (size_t) h,
                8, (size_t) bitmapData.lineStride,
                colourSpace,
                (uint32_t) kCGImageAlphaPremultipliedFirst | (uint32_t) kCGBitmapByteOrder32Little);

            if (context != nullptr)
            {
                CGContextDrawImage (context, CGRectMake (0, 0, w, h), cgImage);
                CGContextRelease (context);
            }

            CGColorSpaceRelease (colourSpace);
        }

        CGImageRelease (cgImage);

        pimpl->addToCache (frameIndex, juceImage);
        return juceImage;
    }
}
