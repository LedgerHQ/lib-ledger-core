{
	'targets': [
        {
            'target_name': 'lib-binding',
            'android_unmangled_name': 1,
            'type': 'shared_library',
            'dependencies': [
                'sources/support-lib/support_lib.gyp:djinni_jni'
            ],
            'ldflags' : [ '-llog' ],
            'sources': [
                '<!@(python glob.py android/jni/jni *.hpp)',
                '<!@(python glob.py android/jni/jni/jni *.hpp *.cpp)',
            ],
            'include_dirs': [
                "android/src/main/include",
                "android/jni/jni",
                "android/jni/jni/jni",
            ],
            'all_dependent_settings': {
                'include_dirs': [
                    "android/src/main/include",
                    "android/jni/jni",
                    "android/jni/jni/jni",
                ],
            },
            'libraries': ['libledger-core.so'],
           'libraries_dir': ['android/jni/prebuilt/'],
        },
    ],
}
