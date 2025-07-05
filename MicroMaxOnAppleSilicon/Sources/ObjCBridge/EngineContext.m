#import <Foundation/Foundation.h>
#import "include/MicroMaxObjCBridge.h"

// Force to terminate the search
int __forceTerminate__ = 0;

void __printToConsole__(const char * str)
{
    NSLog(@"%s", str);
}

void getMicroMaxIni(char *ini) {
    // Determine ObjCBridge resource bundle (try common bundle name and SPM bundle name)
    NSBundle *classBundle = [NSBundle bundleForClass:[MicroMaxObjCBridge class]];
    NSArray<NSString *> *bundleNames = @[@"MicroMaxObjCBridge", @"MicroMaxOnAppleSilicon_MicroMaxObjCBridge"];
    NSBundle *resourceBundle = nil;
    for (NSString *bundleName in bundleNames) {
        NSURL *url = [classBundle URLForResource:bundleName withExtension:@"bundle"];
        if (url) {
            resourceBundle = [NSBundle bundleWithURL:url];
            break;
        }
    }
    if (!resourceBundle) {
        resourceBundle = classBundle;
    }

    // Look for fmax.ini in the determined bundle
    NSString *path = [resourceBundle pathForResource:@"fmax" ofType:@"ini"];
    if (!path) {
        NSLog(@"🔍 fmax.ini not found in bundle: %@", resourceBundle.bundlePath);
        return;
    }

    // Copy the UTF8 path into the provided buffer
    const char *utf8Path = [path UTF8String];
    if (utf8Path) {
        strcpy(ini, utf8Path);
    } else {
        NSLog(@"⚠️ Failed to convert fmax.ini path to UTF8 string.");
    }
}
