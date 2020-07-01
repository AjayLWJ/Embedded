int flag = 0;

for (int i = 0; i <= (M - 1)(M - 1); i++)
{
    if (flag == 0)
    {
        for (int j = 0; j < M; j++)
        {
            for (int k = 0; j < M; k++)
            {
                if (j + k == i)
                {
                     printf("%d", a[j][k]);
                     break;
                }
            }
        }
        flag = 1;
    }
    if (flag == 1)
    {
        for (int j = 0; j < M; j++)
        {
            for (int k = 0; j < M; k++)
            {
                if (j + k == i)
                {
                     printf("%d", a[j][k]);
                     break;
                }
            }
        }
        flag = 0;
    }
}