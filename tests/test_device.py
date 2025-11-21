import sys
from fido2.hid import CtapHidDevice
from fido2.client import Fido2Client, UserInteraction
from fido2.server import Fido2Server
from fido2.webauthn import UserVerificationRequirement, AttestationConveyancePreference
from fido2.ctap1 import Ctap1
from fido2.ctap2 import Ctap2

# Configuration (Match firmware/main/usb_descriptors.c)
VID = 0xCAFE
PID = 0x4000

def find_device():
    print(f"[*] Looking for device VID={hex(VID)} PID={hex(PID)}...")
    devs = list(CtapHidDevice.list_devices())
    for d in devs:
        if d.descriptor.vid == VID and d.descriptor.pid == PID:
            print(f"[+] Found device: {d}")
            return d
    print("[-] Device not found! Make sure it is flashed and connected.")
    return None

def test_u2f(dev):
    print("\n=== Testing U2F (CTAP1) ===")
    try:
        ctap1 = Ctap1(dev)
        print("[*] Sending U2F Register...")
        # Challenge (32 bytes), AppParam (32 bytes)
        challenge = b"A" * 32
        app_param = b"B" * 32
        
        reg_res = ctap1.register(challenge, app_param)
        print("[+] Register Success!")
        print(f"    Public Key: {reg_res.public_key.hex()[:20]}...")
        print(f"    Key Handle: {reg_res.key_handle.hex()}")
        
        print("[*] Sending U2F Authenticate...")
        auth_res = ctap1.authenticate(challenge, app_param, reg_res.key_handle)
        print("[+] Authenticate Success!")
        print(f"    Counter: {auth_res.counter}")
        print(f"    Signature: {auth_res.signature.hex()[:20]}...")
        
    except Exception as e:
        print(f"[-] U2F Test Failed: {e}")

def test_fido2(dev):
    print("\n=== Testing FIDO2 (CTAP2) ===")
    try:
        ctap2 = Ctap2(dev)
        info = ctap2.get_info()
        print(f"[+] GetInfo Success!")
        print(f"    Versions: {info.versions}")
        print(f"    AAGUID: {info.aaguid.hex()}")
        
        if "FIDO_2_0" not in info.versions:
            print("[-] Device does not claim FIDO2 support.")
            return

        # Note: Full MakeCredential requires complex CBOR args which our minimal stack 
        # might struggle with if not perfectly aligned.
        # We'll try a basic MakeCredential if the library allows low-level access,
        # otherwise we just rely on GetInfo for this phase.
        print("[*] FIDO2 Basic checks passed.")
        
    except Exception as e:
        print(f"[-] FIDO2 Test Failed: {e}")

if __name__ == "__main__":
    dev = find_device()
    if dev:
        test_u2f(dev)
        test_fido2(dev)
