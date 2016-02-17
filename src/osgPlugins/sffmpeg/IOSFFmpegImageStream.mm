#include "IOSFFmpegImageStream.hpp"
#include <Photos/Photos.h>
#import <SXCommon/PHAsset+URL.h>

namespace osgFFmpeg {

IOSFFmpegImageStream::IOSFFmpegImageStream() :
    FFmpegImageStream::FFmpegImageStream()
{
}

IOSFFmpegImageStream::IOSFFmpegImageStream(const IOSFFmpegImageStream & image, const osg::CopyOp & copyop) :
    FFmpegImageStream::FFmpegImageStream(image, copyop)
{
}




void IOSFFmpegImageStream::open_finish() {
    if (_stub != nil) {
        return;
    }
    _stub = [NSMutableDictionary new];
    __weak NSMutableDictionary* weakStub = _stub;

    PHVideoRequestOptions* options = [[PHVideoRequestOptions alloc] init];
    options.networkAccessAllowed = NO;
    options.deliveryMode = PHVideoRequestOptionsDeliveryModeHighQualityFormat;

    PHImageManager* manager = [PHImageManager defaultManager];

    NSURL* url = [NSURL URLWithString:[[NSString alloc] initWithUTF8String:_fileName.c_str()]];
    PHAsset* phAsset = [PHAsset phAssetURLParse:url];
    if (phAsset == nil) {
        _stub[@"url"] = [[NSString alloc] initWithUTF8String:_fileName.c_str()];
        return;
    }

    PHImageRequestID requestID = [manager requestAVAssetForVideo:phAsset options:options
                                                   resultHandler:^(AVAsset *originalAsset,
                                                                   AVAudioMix *audioMix,
                                                                   NSDictionary *info) {
                                                       __strong NSMutableDictionary* strongStub = weakStub;
                                                       if (strongStub == nil) {
                                                           return;
                                                       }
                                                       AVAsset* avAsset_ = originalAsset;

                                                       NSArray* tracks = [originalAsset tracksWithMediaType:AVMediaTypeVideo];
                                                       if (tracks != nil){
                                                           // Remove all the Apple magic on slo motion videos
                                                           // But still pass the raw asset if necessary
                                                           for(int i = 0; i < [tracks count] ;i ++) {
                                                               AVAssetTrack* track = tracks[i];
                                                               if ([track.class isSubclassOfClass:[AVCompositionTrack class]]) {
                                                                   NSArray* segments = ((AVCompositionTrack*)track).segments;
                                                                   if ([segments count] > 0 ) {
                                                                       AVCompositionTrackSegment* segment = segments[0];
                                                                       NSURL* url = segment.sourceURL;
                                                                       avAsset_ = [AVURLAsset assetWithURL:url];
                                                                   }
                                                               }
                                                           }
                                                       }

                                                       if ([avAsset_.class isSubclassOfClass:[AVURLAsset class]]) {
                                                           AVURLAsset* _avurlasset = (AVURLAsset*) avAsset_;
                                                           strongStub[@"url"] = [_avurlasset.URL absoluteString];
                                                           strongStub[@"avasset"] = originalAsset;
                                                       }
        }];
}

const char* IOSFFmpegImageStream::get_real_file_name() {
    if (_stub == nil) {
        return NULL;
    }
    NSString* urlString = _stub[@"url"];
    if (urlString != nil) {
        return [urlString UTF8String];
    } else {
        return NULL;
    }
}

void IOSFFmpegImageStream::quit(bool waitForThreadToExit) {
    FFmpegImageStream::quit(waitForThreadToExit);
    _stub = nil;
}

} // namespace osgFFmpeg
