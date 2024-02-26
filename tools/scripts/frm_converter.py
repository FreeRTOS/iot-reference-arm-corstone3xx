#!/usr/bin/env python3
#  Copyright 2024 Arm Limited and/or its affiliates <open-source-office@arm.com>
#  SPDX-License-Identifier: BSD-3-Clause

import click
import cv2
import glob
import os
import numpy as np
from PIL import Image

rggb_modes = ["RG", "GR", "GB", "BG"]


def cv2_bayer_rgb(rggb):
    if rggb == 0:
        # RG
        return cv2.COLOR_BayerRG2RGB
    elif rggb == 1:
        # GB
        return cv2.COLOR_BayerGB2RGB
    elif rggb == 2:
        # GR
        return cv2.COLOR_BayerGR2RGB
    elif rggb == 3:
        # BG
        return cv2.COLOR_BayerBG2RGB
    print("Invalid rggb format {}".format(rggb))
    return 0


def bpp_to_np_dtype(bpp):
    if bpp == 1:
        return np.uint8
    elif bpp == 2:
        return np.uint16
    elif bpp == 4:
        return np.uint32
    else:
        print("Invalid number of bytes per pixel {}".format(bpp))
    return 0


def read_frm(rf):
    desc = rf.readline()
    if not desc:
        return False  # EOF
    width = int(rf.readline())
    height = int(rf.readline())
    depth = int(rf.readline())
    type = int(rf.readline())  # bits per pixel
    bpp = int(type / 8)  # bytes per pixel
    layers = int(rf.readline())
    np_dtype = bpp_to_np_dtype(bpp)
    buffer = rf.read(bpp * width * height * layers)
    data = np.frombuffer(buffer, dtype=np_dtype).reshape(height, width, layers)
    return width, height, depth, type, layers, data


def write_frm(wf, width, height, depth, data_bgr, rggb, container):
    newlayers = 1
    wf.write("#ISP1.width.height.depth.type.layers:\n".encode())
    wf.write(("{:07d}\n").format(width).encode())
    wf.write(("{:07d}\n").format(height).encode())
    wf.write(str(depth).encode())
    wf.write("\n".encode())
    wf.write(str(container).encode())
    wf.write("\n".encode())
    wf.write(("{:03d}\n").format(newlayers).encode())
    bpp = int(container / 8)
    np_dtype = bpp_to_np_dtype(bpp)
    bayer8 = np.empty((height, width, 1), dtype=np.uint8)

    if rggb == 0:
        # RG GB
        bayer8[0::2, 0::2] = data_bgr[0::2, 0::2, 2:3]
        bayer8[0::2, 1::2] = data_bgr[0::2, 1::2, 1:2]
        bayer8[1::2, 0::2] = data_bgr[1::2, 0::2, 1:2]
        bayer8[1::2, 1::2] = data_bgr[1::2, 1::2, 0:1]
    elif rggb == 1:
        # GB RG
        bayer8[0::2, 0::2] = data_bgr[0::2, 0::2, 1:2]
        bayer8[0::2, 1::2] = data_bgr[0::2, 1::2, 0:1]
        bayer8[1::2, 0::2] = data_bgr[1::2, 0::2, 2:3]
        bayer8[1::2, 1::2] = data_bgr[1::2, 1::2, 1:2]
    elif rggb == 2:
        # GR BG
        bayer8[0::2, 0::2] = data_bgr[0::2, 0::2, 1:2]
        bayer8[0::2, 1::2] = data_bgr[0::2, 1::2, 2:3]
        bayer8[1::2, 0::2] = data_bgr[1::2, 0::2, 0:1]
        bayer8[1::2, 1::2] = data_bgr[1::2, 1::2, 1:2]
    elif rggb == 3:
        # BG GR
        bayer8[0::2, 0::2] = data_bgr[0::2, 0::2, 0:1]
        bayer8[0::2, 1::2] = data_bgr[0::2, 1::2, 1:2]
        bayer8[1::2, 0::2] = data_bgr[1::2, 0::2, 1:2]
        bayer8[1::2, 1::2] = data_bgr[1::2, 1::2, 2:3]

    bayer = np.empty((height, width, 1), dtype=np_dtype)
    ls = np.uint32((depth - 8))
    np.left_shift(bayer8, [ls], bayer)
    wf.write(bayer.tobytes())


def raw_to_img(data, width, height, depth, rggb):
    data8 = np.empty((height, width, 1), dtype=np.uint8)
    rs = np.uint32((depth - 8))
    np.right_shift(data, [rs], data8)
    data_rgb = cv2.cvtColor(data8, cv2_bayer_rgb(rggb))
    return data_rgb


@click.command()
@click.option(
    "--input",
    "-i",
    type=click.Path(exists=True),
    required=True,
    help="Path to input file/directory",
)
@click.option(
    "--output",
    "-o",
    type=click.Path(exists=True, file_okay=False),
    default="./",
    show_default=True,
    help="Path to output directory",
)
@click.option(
    "--depth",
    "-d",
    type=click.Choice(["8", "10", "12", "14", "16", "20"]),
    default="12",
    show_default=True,
    help="Output frm bit-depth",
)
@click.option(
    "--container",
    "-c",
    type=click.Choice(["8", "16", "32"]),
    show_default=True,
    help="Output frm container size",
)
@click.option(
    "--bayer",
    "-b",
    type=click.Choice(rggb_modes),
    default="GB",
    show_default=True,
    help="Bayer filter mode",
)
@click.option(
    "--encode/--decode", default=False, help="Encode images to FRM / Decode FRM"
)
@click.option(
    "--from_isp_out",
    is_flag=True,
    default=False,
    help="Decode channel-separated FRM (ISP output)",
)
def main(
    input: str,
    output: str,
    depth: str,
    container: str,
    bayer: str,
    encode: bool,
    from_isp_out: bool,
) -> None:
    """
    Converting frm files to images and vice versa.
    """
    depth = int(depth)
    rggb = rggb_modes.index(bayer)
    if container is None:
        if depth > 16:
            container = 32
        elif depth > 8:
            container = 16
        else:
            container = 8
    else:
        container = int(container)
    print("input: {}".format(input))
    print("output: {}".format(output))
    print("RGGB: {}".format(rggb))
    print("depth: {}".format(depth))
    print("container: {}".format(container))
    print("encode: {}".format(encode))

    if os.path.isdir(input):
        if from_isp_out and not encode:
            inputs = glob.glob(os.path.join(input, "isp_*out_*0_*"))
        else:
            inputs = os.listdir(input)
        basedir = input
    else:
        inputs = [input]
        basedir = "."

    if encode:
        frm_outfile = os.path.join(output, os.path.basename(inputs[0]) + ".conv.frm")
        print("Output: {}".format(frm_outfile))
        with open(frm_outfile, "wb") as wf:
            for file in inputs:
                data_bgr = np.asarray(Image.open(os.path.join(basedir, file)))
                height, width, layers = data_bgr.shape
                print(
                    "Input: {}: {}x{}@{}/{} L{}".format(
                        file, width, height, 24, 24, layers
                    )
                )
                write_frm(wf, width, height, depth, data_bgr, rggb, container)
    elif from_isp_out:
        for file0 in inputs:
            file1 = file0.replace("ds0", "ds1").replace("fr0", "fr1")
            file2 = file0.replace("ds0", "ds2").replace("fr0", "fr2")
            with open(file0, "rb") as rf0:
                with open(file1, "rb") as rf1:
                    with open(file2, "rb") as rf2:
                        counter = 0
                        img_outfile = os.path.join(
                            output, os.path.basename(file0) + ".{}.bmp"
                        )
                        while True:
                            result = read_frm(rf0)
                            if not result:
                                break
                            width, height, depth, type, layers, data0 = result
                            _, _, _, _, _, data1 = read_frm(rf1)
                            _, _, _, _, _, data2 = read_frm(rf2)
                            rgb0 = raw_to_img(data0, width, height, depth, rggb)
                            rgb1 = raw_to_img(data1, width, height, depth, rggb)
                            rgb2 = raw_to_img(data2, width, height, depth, rggb)
                            data8 = np.empty((height, width, 3), dtype=np.uint8)
                            data8[0::1, 0::1, 0:1] = rgb0[0::1, 0::1, 0:1]
                            data8[0::1, 0::1, 1:2] = rgb1[0::1, 0::1, 0:1]
                            data8[0::1, 0::1, 2:3] = rgb2[0::1, 0::1, 0:1]
                            image = Image.fromarray(data8)
                            print(
                                "Output from {}x{}@{}/{} L{}: {}".format(
                                    width,
                                    height,
                                    depth,
                                    type,
                                    layers,
                                    img_outfile.format(counter),
                                )
                            )
                            image.save(img_outfile.format(counter))
                            counter = counter + 1
    else:
        for file in inputs:
            with open(os.path.join(basedir, file), "rb") as rf:
                counter = 0
                img_outfile = os.path.join(output, os.path.basename(file) + ".{}.bmp")
                while True:
                    result = read_frm(rf)
                    if not result:
                        break
                    width, height, depth, type, layers, data = result
                    if layers == 1:
                        data8 = raw_to_img(data, width, height, depth, rggb)
                    else:
                        data8 = np.empty((height, width, 3), dtype=np.uint8)
                        rs = np.uint32((depth - 8))
                        np.right_shift(data, [rs, rs, rs], data8)
                    image = Image.fromarray(data8)
                    print(
                        "Output from {}x{}@{}/{} L{}: {}".format(
                            width,
                            height,
                            depth,
                            type,
                            layers,
                            img_outfile.format(counter),
                        )
                    )
                    image.save(img_outfile.format(counter))
                    counter = counter + 1


if __name__ == "__main__":
    main()
