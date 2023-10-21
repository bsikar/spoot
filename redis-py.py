import redis


def show_redis_contents():
    # Connect to local Redis server
    r = redis.StrictRedis(host="127.0.0.1", port=6379, db=0)

    # Retrieve all fields and values from the 'user_colors' hash
    user_colors = r.hgetall("user_colors")

    for user, color in user_colors.items():
        print(f"{user.decode('utf-8')}: {color.decode('utf-8')}")


if __name__ == "__main__":
    show_redis_contents()
