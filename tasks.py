from invoke import task
import json
import os
import hashlib
from datetime import datetime
import zipfile


@task(name="build-firmware")
def build_firmware(c):
    builds = [
        ("platformio/nahs-LatchBrick_v1", "esp12e")
    ]
    generated = list()

    for base_dir, pio_env in builds:
        os.environ['PATH'] = "/home/nijo/.platformio/penv/bin:/home/nijo/.platformio/penv:/home/nijo/.platformio/python3/bin:" + os.environ.get('PATH')
        c.run("rm -rf " + os.path.join(base_dir, '.pio'))
        c.run("cd " + base_dir + "; pio pkg install")
        c.run("cd " + base_dir + "; pio run --environment " + pio_env)

        metadata = dict()

        # set build as development version
        metadata['dev'] = False

        # get brick_type
        for f in os.listdir(os.path.join(base_dir, 'src')):
            if f.startswith('main'):
                with open(os.path.join(base_dir, 'src', f)) as f:
                    for line in f.read().strip().split('\n'):
                        if 'setBrickType' in line:
                            metadata['brick_type'] = int(line.rsplit('(', 1)[1].split(')', 1)[0])
                            break
                break

        # get version
        metadata['version'] = datetime.now().strftime("%Y%m%d%H%M")

        # calculate sketchMD5
        sketchMD5 = hashlib.md5()
        with open(os.path.join(base_dir, '.pio/build', pio_env, 'firmware.bin'), "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                sketchMD5.update(chunk)
        metadata['sketchMD5'] = sketchMD5.hexdigest()

        # collecting version info of libdeps
        libs_dir = os.path.join(base_dir, '.pio/libdeps', pio_env)
        metadata['content'] = dict()
        for d in os.listdir(libs_dir):
            library_json = os.path.join(libs_dir, d, 'library.json')
            if os.path.isfile(library_json):
                with open(library_json, 'r') as f:
                    v = json.load(f)['version']
                metadata['content'][d] = v

        # write out zipfile
        fw_name = 'bfw_' + str(metadata['brick_type']) + '_' + metadata['version'] + ('_dev' if 'dev' in metadata and metadata['dev'] else '') + '.zip'
        with zipfile.ZipFile(fw_name, 'w') as zf:
            zf.writestr('metadata.json', json.dumps(metadata, indent=2))
            zf.write(os.path.join(base_dir, '.pio/build', pio_env, 'firmware.bin'), 'firmware.bin')
        generated.append(fw_name)

    print('')
    for g in generated:
        print('Generated: ' + g)
