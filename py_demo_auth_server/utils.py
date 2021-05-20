import argon2

def VerifyPassword(passhash, plaintext):
    try:
        ph = argon2.PasswordHasher()
        ph.verify(passhash, plaintext)
    except argon2.exceptions.VerificationError as e:
        return False
    return True
