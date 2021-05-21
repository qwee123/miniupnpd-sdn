import OpenSSL
import argon2
import base64
import json
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
    \"pub_port_range\": \"$pub_port_range\",
    \"int_ip_range\": \"$int_ip_range\",
    \"public_key\": \"$pub_key\"
}
"""

def GenerateToken(perm_str, pubkey, signkey):
    proof_tp = Template(token_template)

    perm = json.loads(perm_str)

    try:
        proof_value = {
            'int_ip_range': perm['int_ip_range'],
            'pub_port_range': perm['pub_port_range'],
        }
    except KeyError as e:
        print(f'token field {e} is not provided.')
    proof_value['pub_key'] = pubkey

    #Each field mentioned in the template is guaranteed to be provided here.
    proof = proof_tp.substitute(proof_value)

    header = "{\"alg\": \"RS256\"}"

    headerb64 = base64.b64encode(header.encode(encoding="utf-8"))
    proofb64 = base64.b64encode(proof.encode(encoding="utf-8"))

    payload = header + proof
    pkey = OpenSSL.crypto.load_privatekey(OpenSSL.crypto.FILETYPE_PEM, signkey)
    sig = OpenSSL.crypto.sign(pkey, payload, "sha256")
    sigb64 = base64.b64encode(sig)

    token = headerb64 + b"." + proofb64 + b"." + sigb64

    return token
