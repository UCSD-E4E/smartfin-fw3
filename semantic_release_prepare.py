'''Prepares the version information
'''
import argparse
import re
from pathlib import Path

import semantic_version


def main():
    """Places the version number into the version file
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('version')
    parser.add_argument('branch')
    parser.add_argument('vers_header', type=Path)
    args = parser.parse_args()
    version = semantic_version.Version(args.version)
    version_header = args.vers_header


    major_regex = r'#define FW_MAJOR_VERSION\s*\d*'
    minor_regex = r'#define FW_MINOR_VERSION\s*\d*'
    patch_regex = r'#define FW_BUILD_NUM\s*\d*'
    major_subst = f'#define FW_MAJOR_VERSION    {version.major}'
    minor_subst = f'#define FW_MINOR_VERSION    {version.minor}'
    patch_subst = f'#define FW_BUILD_NUM        {version.patch}'

    new_lines = []
    with open(version_header, 'r', encoding='utf-8') as handle:
        for line in handle:
            line = re.sub(major_regex, major_subst, line)
            line = re.sub(minor_regex, minor_subst, line)
            line = re.sub(patch_regex, patch_subst, line)
            new_lines.append(line)

    with open(version_header, 'w', encoding='utf-8') as handle:
        handle.writelines(new_lines)

if __name__ == '__main__':
    main()
