#import <Foundation/Foundation.h>
#import "include/MicroMaxObjCBridge.h"

// Force to terminate the search
int __forceTerminate__ = 0;

void __printToConsole__(const char * str)
{
    NSLog(@"%s", str);
}

void getMicroMaxIni(char *ini) {
    /// The bundle for the MicroMaxBridge class
    NSBundle *bundle = [NSBundle bundleForClass:[MicroMaxObjCBridge class]];
    
    /// Try to find the `fmax.ini` resource within this bundle
    NSString *path = [bundle pathForResource:@"fmax" ofType:@"ini"];
    
    /// If the path is `nil`, the `fmax.ini` file was not found
    if (!path) {
        /// For Swift Tests we're going to look into two more spots
        NSMutableString *bundlePathAsString1 = [[bundle bundlePath] mutableCopy];
      
        /// First try appending
        [bundlePathAsString1 appendString:@"/MicroMaxOnAppleSilicon_MicroMaxObjCBridge.bundle"];

        NSBundle *bundleFirstTry = [NSBundle bundleWithPath:bundlePathAsString1];
        path = [bundleFirstTry pathForResource:@"fmax" ofType:@"ini"];
        
        if (!path) {
            /// Second try replacing
            NSMutableString *bundlePathAsString2 = [[bundle bundlePath] mutableCopy];
            [bundlePathAsString2 replaceOccurrencesOfString:@"/MicroMaxOnAppleSiliconPackageTests.xctest"
                                                 withString:@"/MicroMaxOnAppleSilicon_MicroMaxObjCBridge.bundle"
                                                    options:NSBackwardsSearch
                                                      range:NSMakeRange(0, [bundlePathAsString2 length])];
            NSBundle *bundleSecondTry = [NSBundle bundleWithPath:bundlePathAsString2];
            path = [bundleSecondTry pathForResource:@"fmax" ofType:@"ini"];
        }
    }
    /// If the path is `nil`, the `fmax.ini` file was not found
    if (path) {
        const char *pathUTF8 = [path UTF8String];
        if (pathUTF8) {
            strcpy(ini, pathUTF8); // Only copy if pathUTF8 is not NULL
        } else {
            // Handle the error: pathUTF8 is NULL
            NSLog(@"Failed to convert path to UTF8 string.");
        }
    } else {
        // Handle the case where the path is not found
        NSLog(@"fmax.ini not found in the framework bundle.");
    }
}
