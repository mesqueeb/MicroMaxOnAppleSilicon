#import <Foundation/Foundation.h>
#import "include/MicroMaxObjCBridge.h"

// Force to terminate the search
int __forceTerminate__ = 0;

void __printToConsole__(const char * str)
{
    NSLog(@"%s", str);
}

void getMicroMaxIni(char *ini) {
    // 1. Check host app bundle for fmax.ini
    NSBundle *hostBundle = [NSBundle mainBundle];
    NSString *hostPath = [hostBundle pathForResource:@"fmax" ofType:@"ini"];
    if (hostPath) {
        const char *utf8HostPath = [hostPath UTF8String];
        if (utf8HostPath) {
            strcpy(ini, utf8HostPath);
            return;
        }
    }

    // 2. Determine ObjCBridge resource bundle (try common bundle name and SPM bundle name)
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
    if (path) {
        const char *utf8Path = [path UTF8String];
        if (utf8Path) {
            strcpy(ini, utf8Path);
            return;
        } else {
            NSLog(@"⚠️ Failed to convert fmax.ini path to UTF8 string.");
            abort(); // Immediately terminate if conversion fails
        }
    }

    // 3. Final fallback: project resources folder for swift test
    NSString *cwd = [[NSFileManager defaultManager] currentDirectoryPath];
    NSArray<NSString *> *fallbackPaths = @[
        @"MicroMaxOnAppleSilicon/Sources/ObjCBridge/Resources/fmax.ini",
        @"Sources/ObjCBridge/Resources/fmax.ini"
    ];
    
    for (NSString *fallbackPath in fallbackPaths) {
        NSString *fullPath = [cwd stringByAppendingPathComponent:fallbackPath];
        if ([[NSFileManager defaultManager] fileExistsAtPath:fullPath]) {
            const char *utf8Path = [fullPath UTF8String];
            if (utf8Path) {
                strcpy(ini, utf8Path);
                return;
            }
        }
    }

    NSLog(@"🔍 fmax.ini not found in any bundle or fallback path.");
    abort();
}
