import OpenSSL
import argon2
import base64
from string import Template

def VerifyPassword(passhash, plaintext):
    try:
        ph = argon2.PasswordHasher()
        ph.verify(passhash, plaintext)
    except argon2.exceptions.VerificationError as e:
        return False
    return True

token_template = """
{
    \"authorized_port_range\": \"$port_range\",
    \"public_key\": \"$pub_key\"
}
"""

def GenerateToken(pubkey, signkey):
    proof_tp = Template(token_template)

    proof_value = {
        'port_range': '1024-60000',
        'pub_key': pubkey
    }
    try:
        proof = proof_tp.substitute(proof_value)
    except KeyError as e:
        print(f'Value of filed {e} is not provided.')
        return ""

    header = "{\"alg\": \"RS256\"}"

    headerb64 = base64.b64encode(header.encode(encoding="utf-8"))
    proofb64 = base64.b64encode(proof.encode(encoding="utf-8"))

    payload = header + proof
    pkey = OpenSSL.crypto.load_privatekey(OpenSSL.crypto.FILETYPE_PEM, signkey)
    sig = OpenSSL.crypto.sign(pkey, payload, "sha256")
    sigb64 = base64.b64encode(sig)

    token = headerb64 + b"." + proofb64 + b"." + sigb64

    return token
