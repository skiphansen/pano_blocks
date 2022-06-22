#!/bin/bash
export PYTHONHASHSEED=1
exec `dirname $0`/gtkwave-sigrok-filter.py 16.666666 -P usb_signalling:signalling=full-speed:dp=/usb_dp_IBUF:dm=/usb_dm_IBUF,usb_packet:signalling=full-speed,usb_request
